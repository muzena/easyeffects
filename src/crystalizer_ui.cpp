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

#include "crystalizer_ui.hpp"

CrystalizerUi::CrystalizerUi(BaseObjectType* cobject,
                             const Glib::RefPtr<Gtk::Builder>& builder,
                             const std::string& schema,
                             const std::string& schema_path)
    : Gtk::Box(cobject), PluginUiBase(builder, schema, schema_path) {
  name = plugin_name::crystalizer;

  // loading builder widgets

  bands_box = builder->get_widget<Gtk::Box>("bands_box");

  // gsettings bindings

  build_bands(13);

  setup_input_output_gain(builder);
}

CrystalizerUi::~CrystalizerUi() {
  util::debug(name + " ui destroyed");
}

auto CrystalizerUi::add_to_stack(Gtk::Stack* stack, const std::string& schema_path) -> CrystalizerUi* {
  const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/crystalizer.ui");

  auto* const ui = Gtk::Builder::get_widget_derived<CrystalizerUi>(
      builder, "top_box", "com.github.wwmm.easyeffects.crystalizer", schema_path + "crystalizer/");

  stack->add(*ui, plugin_name::crystalizer);

  return ui;
}

void CrystalizerUi::reset() {
  bypass->set_active(false);

  settings->reset("input-gain");

  settings->reset("output-gain");

  for (int n = 0; n < 13; n++) {
    const auto bandn = "band" + std::to_string(n);

    settings->reset("intensity-" + bandn);

    settings->reset("mute-" + bandn);

    settings->reset("bypass-" + bandn);
  }
}

void CrystalizerUi::build_bands(const int& nbands) {
  for (int n = 0; n < nbands; n++) {
    const auto builder = Gtk::Builder::create_from_resource("/com/github/wwmm/easyeffects/ui/crystalizer_band.ui");

    auto* band_box = builder->get_widget<Gtk::Box>("band_box");

    auto* band_label = builder->get_widget<Gtk::Label>("band_label");

    auto* band_intensity = builder->get_widget<Gtk::Scale>("band_intensity");

    auto* band_bypass = builder->get_widget<Gtk::ToggleButton>("band_bypass");
    auto* band_mute = builder->get_widget<Gtk::ToggleButton>("band_mute");

    prepare_scale(band_intensity);

    // connections

    connections.push_back(band_mute->signal_toggled().connect([=]() {
      if (band_mute->get_active()) {
        band_intensity->set_sensitive(false);
      } else {
        band_intensity->set_sensitive(true);
      }
    }));

    const auto bandn = "band" + std::to_string(n);

    settings->bind("intensity-" + bandn, band_intensity->get_adjustment().get(), "value");
    settings->bind("mute-" + bandn, band_mute, "active");
    settings->bind("bypass-" + bandn, band_bypass, "active");

    switch (n) {
      case 0:
        band_label->set_text("250 Hz");

        break;
      case 1:
        band_label->set_text("750 Hz");

        break;
      case 2:
        band_label->set_text("1.5 kHz");

        break;
      case 3:
        band_label->set_text("2.5 kHz");

        break;
      case 4:
        band_label->set_text("3.5 kHz");

        break;

      case 5:
        band_label->set_text("4.5 kHz");

        break;
      case 6:
        band_label->set_text("5.5 kHz");

        break;
      case 7:
        band_label->set_text("6.5 kHz");

        break;
      case 8:
        band_label->set_text("7.5 kHz");

        break;
      case 9:
        band_label->set_text("8.5 kHz");

        break;
      case 10:
        band_label->set_text("9.5 kHz");

        break;
      case 11:
        band_label->set_text("12.5 kHz");

        break;
      case 12:
        band_label->set_text("17.5 kHz");

        break;
    }

    bands_box->append(*band_box);
  }

  bands_box->show();
}
