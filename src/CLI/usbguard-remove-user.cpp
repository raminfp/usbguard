//
// Copyright (C) 2017 Red Hat, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Authors: Daniel Kopecek <dkopecek@redhat.com>
//
#include "usbguard.hpp"
#include "usbguard-remove-user.hpp"

#include <iostream>
#include <unistd.h>

namespace usbguard
{
  static const char *options_short = "h";

  static const struct ::option options_long[] = {
    { "help", no_argument, nullptr, 'h' },
    { nullptr, 0, nullptr, 0 }
  };

  static void showHelp(std::ostream& stream)
  {
    stream << " Usage: " << usbguard_arg0 << " remove-user [OPTIONS]" << std::endl;
    stream << std::endl;
    stream << " Options:" << std::endl;
    stream << "  -h, --help  Show this help." << std::endl;
    stream << std::endl;
  }

  int usbguard_remove_user(int argc, char *argv[])
  {
    int opt = 0;

    while ((opt = getopt_long(argc, argv, options_short, options_long, nullptr)) != -1) {
      switch(opt) {
        case 'h':
          showHelp(std::cout);
          return EXIT_SUCCESS;
        case '?':
          showHelp(std::cerr);
        default:
          return EXIT_FAILURE;
      }
    }

    return EXIT_SUCCESS;
  }
} /* namespace usbguard */
