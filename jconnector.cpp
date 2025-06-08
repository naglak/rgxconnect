/*
 * Copyright (C) 2023, 2024 Jacek Naglak
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "jconnector.h"
#include <regex>

JackConnector::JackConnector(const char *clientname, const char *servername) {
  jack_status_t status;
  jack_options_t options = JackNoStartServer;
  if (servername)
    options = static_cast<jack_options_t>(JackNoStartServer | JackServerName);
  client = jack_client_open(clientname, options, &status, servername);
  if (client == 0) {
    fprintf(stderr, "Error connecting to jack server.\n");
    exit(EXIT_FAILURE);
  }
  jack_set_port_registration_callback(client, &jack_reg_callback, this);
  jack_on_shutdown(client, jack_shutdown_callback, 0);
  if (jack_activate(client)) {
    fprintf(stderr, "Cannon activate client.\n");
    exit(EXIT_FAILURE);
  }
}

JackConnector::~JackConnector() { jack_client_close(client); }

void JackConnector::add_rule(string s1, string s2) {
  key1.push_back(s1);
  key2.push_back(s2);
}

void JackConnector::launch() {
  thread = std::thread(&JackConnector::run_thread, this);
  thread.join();
}

void JackConnector::jack_reg_callback(jack_port_id_t port, int registered,
                                      void *arg) {
  if (registered == 0)
    return;
  JackConnector *this_ = static_cast<JackConnector *>(arg);
  this_->jack_reg_callback_prv(port, 0);
}

void JackConnector::jack_shutdown_callback(void *arg) { exit(EXIT_FAILURE); }

void *JackConnector::run_thread(void *arg) {
  JackConnector *this_ = static_cast<JackConnector *>(arg);
  jack_port_id_t port;

  while (1) {
    std::unique_lock<std::mutex> thread_lock(this_->thread_mutex);
    this_->thread_cv.wait(thread_lock, [this_] { return this_->thread_guard; });
    this_->thread_guard = false;
    port = this_->jack_port;
    this_->connect(port);
    this_->callback_guard = true;
    this_->callback_cv.notify_one();
  }
  return this_;
}

void JackConnector::jack_reg_callback_prv(jack_port_id_t port, void *arg) {
  {
    std::unique_lock<std::mutex> thread_lock(thread_mutex);
    jack_port = port;
    thread_guard = true;
  }
  thread_cv.notify_one();
  std::unique_lock<std::mutex> callback_lock(callback_mutex);
  callback_cv.wait(callback_lock, [this] { return callback_guard; });
  callback_guard = false;
}

void JackConnector::connect(jack_port_id_t port) {
  const char **ports;
  jack_port_t *port_id = jack_port_by_id(client, port);
  string port_name = jack_port_name(port_id);
  string ports_regex;

  for (vector<string>::size_type i = 0; i < key1.size(); i++) {
    std::regex reg{key1[i]};
    if (regex_search(port_name, reg)) {
      ports_regex = key2[i];
      if (jack_port_flags(port_id) & JackPortIsOutput) {
        ports =
            jack_get_ports(client, ports_regex.c_str(), NULL, JackPortIsInput);
        if (ports) {
          for (size_t j = 0; ports[j]; j++) {
            if (!jack_port_connected_to(port_id, ports[j]))
              jack_connect(client, port_name.c_str(), ports[j]);
          }
          jack_free(ports);
        }
      } else if (jack_port_flags(port_id) & JackPortIsInput) {
        ports =
            jack_get_ports(client, ports_regex.c_str(), NULL, JackPortIsOutput);
        if (ports) {
          for (size_t j = 0; ports[j]; j++) {
            if (!jack_port_connected_to(port_id, ports[j]))
              jack_connect(client, ports[j], port_name.c_str());
          }
          jack_free(ports);
        }
      }
    }
  }
}
