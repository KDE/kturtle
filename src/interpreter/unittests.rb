#!/usr/bin/ruby

#  Copyright (C) 2006 by Cies Breijs
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of version 2 of the GNU General Public
#  License as published by the Free Software Foundation.
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


require 'open3'


def split_in_parts(chunk)
	ntr = ''
	exe = ''
	sig = ''
	chunk.each_line do |l|
		case l[0..3]
			when 'NTR>'
				ntr += l
			when 'EXE>'
				exe += l
			when 'SIG>'
				sig += l
		end
	end
	return {'NTR' => ntr, 'EXE' => exe, 'SIG' => sig}
end

def get_test_goal(test_file_name)
	f = open test_file_name
	result = ''
	f.each_line do |l|
		result += l[2..-1] ? l[2..-1] : ''
		result = '' if l =~ /^### TEST GOAL/
	end
	split_in_parts(result)
end

def get_test_results(kturtle_exe, test_file_name)
	(stdin, stdout, stderr) = Open3.popen3("#{kturtle_exe} --test #{test_file_name}")
	split_in_parts(stderr)
end

# def difference(file_name, string)
# 	tmp_name = ".generate.tmp"
# 	gen_file = File.new(tmp_name, "w")
# 	gen_file.write(string)
# 	gen_file.close()
# 
# 	system("diff #{file_name} #{tmp_name}")
# end




puts <<EOS

unit testing script for the interpreter of KTurtle
by Cies Breijs

Usage: ruby unittests.rb path/to/kturtle path/to/dir/containing/unittests/ [-q]
('-q' enables the quick-fix mechanism)

EOS

unless [2,3].include? ARGV.length
	puts 'Wrong amount of arguments specified... Exitting.'
	exit
end

kturtle_exe   = ARGV[0]
unittests_dir = ARGV[1]
quick_fix     = ARGV[2] == '-q'

tests = Dir[unittests_dir + '*.turtle']

puts "Running the tests...\n\n"

i = 1
tests.each do |test_file_name|
	puts "### test #{i.to_s}: #{test_file_name}"
	goal    = get_test_goal(test_file_name)
	results = get_test_results(kturtle_exe, test_file_name)

	success = true
	goal.each_key do |k|
		if goal[k] == results[k]
			puts "#{k} OK"
		else
			puts "\n\n#{test_file_name} #{k} test FAILED\n"
			puts "### got:"
			puts results[k]
			puts "\n"
			puts "### needed:"
			puts goal[k]
			success = false
		end
	end

	if not success and quick_fix
		puts "QuickFix (tm) this failure?"
		puts "(press 'y' and ENTER for a QuickFix, any other input will _not_ QuickFix)"
		input = gets.strip
		if input == 'y'
			old_test = open(test_file_name).read
			new_test = ''
			old_test.each_line do |l|
				break if l =~ /^### TEST GOAL/
				new_test += l
			end
			new_test += "### TEST GOAL\n"
			results.each_key { |k|  results[k].each_line { |l| new_test += "# #{l}" } }
			f = File.new(test_file_name, 'w')
			f.write(new_test)
			f.close
			puts "fixed..."
		else
			puts "not fixed..."
		end
	end

	i += 1
end



