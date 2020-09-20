#!/usr/bin/env ruby
#  SPDX-FileCopyrightText: 2009 Niels Slot
#
#  SPDX-License-Identifier: GPL-2.0-or-later

puts "Killing interpreter process."

pid = IO.readlines("pid")[0].to_i
File.delete("pid")

Process.kill('TERM', pid)