#!/usr/bin/env ruby
puts "Killing interpreter process."

pid = IO.readlines("pid")[0].to_i
File.delete("pid")

Process.kill('TERM', pid)