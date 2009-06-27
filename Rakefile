require 'rubygems'
require 'rake'
require 'rake/testtask'
require 'fileutils'
include FileUtils

desc "Find all TODO's"
task :todo do
  FileList['**/*.rb'].egrep(/TODO/)
end

require 'spec/rake/spectask'

desc "Run all specs"
Spec::Rake::SpecTask.new('spectask') do |t|
  t.spec_opts = ["--colour"]
  t.spec_files = FileList['spec/**/*.rb']
end

task :spec do
  # this is not too nice as we hide all the nice features of SpecTask
  # yet i really wanted to take the starting and stopping of the
  # interpreter out of the spec files and in to the Rakefile.
  puts "Staring the KTurtle interpreter in DBUS mode..."
  k = IO.popen './src/kturtle --dbus'
  sleep 1.5  # give it some time to start
  ENV['KTURTLE_INTERPRETER_DBUS_PID'] = k.pid.to_s
  puts "started with pid: #{k.pid}"
  begin
    Rake::Task[:spectask].execute
  rescue
    # this is necessary for otherwise the spectask aborts rake alltogether...
  end
  puts "Killing interpreter process."
  Process.kill('TERM', k.pid)
end


task :default => 'spec'
