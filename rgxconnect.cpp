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
#include <fstream>
#include <regex>
#include <unistd.h>

using namespace std;

// erases escape character "\" from entries in a regex file
// allowing use escaped "<" and ">" characters

void erase_escape(std::string &s, std::string escape) {

  long unsigned int pos = 0;

  pos = s.find(escape);
  while (pos != std::string::npos) {
    s.erase(pos, 1);
    pos = s.find(escape);
  }
}

int main(int argc, char *argv[]) {

  const char *clientname = "rgxconnect";
  const char *servername = nullptr;
  std::regex reg{"^\\s*<(.+)>\\s*<(.+)>\\s*$"};
  std::smatch sm;
  int opt = 0;

  while ((opt = getopt(argc, argv, "s:j:h")) != -1) {
    switch (opt) {
    case 's':
      servername = optarg;
      break;
    case 'j':
      clientname = optarg;
      break;
    case 'h':
    default:
      fprintf(stderr, "Usage: %s [-s servername] [-j clientname] regex_file\n",
              argv[0]);
      exit(EXIT_FAILURE);
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s [-s servername] [-j clientname] regex_file\n",
            argv[0]);
    exit(EXIT_FAILURE);
  }

  string regfilename = argv[optind];
  ifstream regfile;

  std::vector<string> rules_2;
  std::vector<string> rules_1;

  regfile.open(regfilename);
  if (regfile.is_open()) {
    std::string buff;
    while (getline(regfile, buff)) {
      while (regex_search(buff, sm, reg)) {
        if (sm.size() == 3) {
          std::string key_1 = sm[1];
          std::string key_2 = sm[2];
          erase_escape(key_1, "\\>");
          erase_escape(key_2, "\\>");
          erase_escape(key_1, "\\<");
          erase_escape(key_2, "\\<");
          rules_1.push_back(key_1);
          rules_2.push_back(key_2);
        }
        buff = sm.suffix();
      }
    }
    regfile.close();
  } else {
    fprintf(stderr, "Can't open regex file\n");
    exit(EXIT_FAILURE);
  }

  JackConnector connector(clientname, servername);

  for (vector<string>::size_type i = 0; i < rules_2.size(); i++) {
    connector.add_rule(rules_1[i], rules_2[i]);
  }

  connector.launch();
  return EXIT_SUCCESS;
}
