#!/usr/bin/env ruby
#  Copyright (C) 2009 by Cies Breijs
#  Copyright (C) 2009 by Niels Slot
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public
#  License as published by the Free Software Foundation; either
#  version 2 of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public
#  License along with this program; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#  Boston, MA 02110-1301, USA.

puts "Staring the KTurtle interpreter in DBUS mode..."
k = IO.popen './src/kturtle --dbus'
sleep 1.5  # give it some time to start
pid_file = File.new("pid", "w")
pid_file.puts(k.pid.to_s)
pid_file.close

puts "started with pid: #{k.pid}"
