/*
 *  Copyright © 2017-2022 Wellington Wallace
 *
 *  This file is part of EasyEffects.
 *
 *  EasyEffects is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  EasyEffects is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with EasyEffects.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "spectrum.hpp"

Spectrum::Spectrum(const std::string& tag,
                   const std::string& schema,
                   const std::string& schema_path,
                   PipeManager* pipe_manager)
    : PluginBase(tag, "spectrum", schema, schema_path, pipe_manager) {
  real_input.resize(n_bands);
  output.resize(n_bands / 2U + 1U);

  complex_output = fftwf_alloc_complex(n_bands);

  plan = fftwf_plan_dft_r2c_1d(static_cast<int>(n_bands), real_input.data(), complex_output, FFTW_ESTIMATE);

  fftw_ready = true;

  settings->signal_changed("show").connect([=, this](const auto& key) {
    std::scoped_lock<std::mutex> lock(data_mutex);

    post_messages = settings->get_boolean(key);
  });
}

Spectrum::~Spectrum() {
  if (connected_to_pw) {
    disconnect_from_pw();
  }

  std::scoped_lock<std::mutex> lock(data_mutex);

  fftw_ready = false;

  if (complex_output != nullptr) {
    fftwf_free(complex_output);
  }

  fftwf_destroy_plan(plan);

  util::debug(log_tag + name + " destroyed");
}

void Spectrum::setup() {
  notification_dt = 0.0F;

  fft_buffer_duration = static_cast<float>(n_bands) / static_cast<float>(rate);
}

void Spectrum::process(std::span<float>& left_in,
                       std::span<float>& right_in,
                       std::span<float>& left_out,
                       std::span<float>& right_out) {
  std::scoped_lock<std::mutex> lock(data_mutex);

  std::copy(left_in.begin(), left_in.end(), left_out.begin());
  std::copy(right_in.begin(), right_in.end(), right_out.begin());

  if (!post_messages || !fftw_ready) {
    return;
  }

  uint count = 0U;

  for (uint n = 0U; n < left_in.size(); n++, count++) {
    if (const uint k = total_count + n; k < real_input.size()) {
      // https://en.wikipedia.org/wiki/Hann_function

      const float w = 0.5F * (1.0F - std::cos(2.0F * std::numbers::pi_v<float> * static_cast<float>(k) /
                                              static_cast<float>(real_input.size() - 1U)));

      real_input[k] = 0.5F * (left_in[n] + right_in[n]) * w;
    } else {
      break;
    }
  }

  total_count += count;

  if (total_count == real_input.size()) {
    total_count = 0U;

    fftwf_execute(plan);

    for (uint i = 0U; i < output.size(); i++) {
      float sqr = complex_output[i][0] * complex_output[i][0] + complex_output[i][1] * complex_output[i][1];

      sqr /= static_cast<float>(n_samples * n_samples);

      output[i] = sqr;
    }
  }

  notification_dt += fft_buffer_duration;

  if (notification_dt >= notification_time_window) {
    auto output_copy = output;

    notification_dt = 0.0F;

    Glib::signal_idle().connect_once([=, this] { power.emit(rate, output_copy.size(), output_copy); });
  }
}
