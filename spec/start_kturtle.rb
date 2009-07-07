#!/usr/bin/env ruby
puts "Staring the KTurtle interpreter in DBUS mode..."
k = IO.popen './src/kturtle --dbus'
sleep 1.5  # give it some time to start
pid_file = File.new("pid", "w")
pid_file.puts(k.pid.to_s)
pid_file.close

puts "started with pid: #{k.pid}"
