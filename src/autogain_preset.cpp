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

#include "autogain_preset.hpp"

AutoGainPreset::AutoGainPreset() {
  input_settings = Gio::Settings::create("com.github.wwmm.easyeffects.autogain",
                                         "/com/github/wwmm/easyeffects/streaminputs/autogain/");

  output_settings = Gio::Settings::create("com.github.wwmm.easyeffects.autogain",
                                          "/com/github/wwmm/easyeffects/streamoutputs/autogain/");
}

void AutoGainPreset::save(nlohmann::json& json,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  json[section]["autogain"]["input-gain"] = settings->get_double("input-gain");

  json[section]["autogain"]["output-gain"] = settings->get_double("output-gain");

  json[section]["autogain"]["target"] = settings->get_double("target");

  json[section]["autogain"]["reference"] = settings->get_string("reference").c_str();
}

void AutoGainPreset::load(const nlohmann::json& json,
                          const std::string& section,
                          const Glib::RefPtr<Gio::Settings>& settings) {
  update_key<double>(json.at(section).at("autogain"), settings, "input-gain", "input-gain");

  update_key<double>(json.at(section).at("autogain"), settings, "output-gain", "output-gain");

  update_key<double>(json.at(section).at("autogain"), settings, "target", "target");

  update_string_key(json.at(section).at("autogain"), settings, "reference", "reference");
}
