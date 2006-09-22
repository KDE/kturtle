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

UNITTESTS_DIR = './unittests/'
RESULTS_TMP_FILE = UNITTESTS_DIR + '.test_results_dump.tmp'


def get_test_goal(test_file_name)
	f = open test_file_name

	result = nil
	f.each_line do |l|
		result += l[2..-1] unless result.nil?
		result = '' if l =~ /^### TEST GOAL/
	end
	
	result
end

def get_test_results(test_file_name)
	system("./interpreter #{test_file_name} #{RESULTS_TMP_FILE} 2>/dev/null")
	open(RESULTS_TMP_FILE).read
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

	UnitTesting script for the interpreter of KTurtle
	by Cies Breijs
	
	This will run some the tests in: #{UNITTESTS_DIR}*.logo
	Tests should contain their own test goals as commented
	out lines on the end of the file following a line staring
	with '### TEST GOAL'.
	
	When the flag '-q' is specified it will not ask for QuickFixes.


Running the tests...

EOS

quick_fix = ARGV[0] != '-q'

tests = Dir[UNITTESTS_DIR + '*.logo']

tests.each do |test_file_name|
	goal = get_test_goal(test_file_name)
	results = get_test_results(test_file_name)

	if goal == results
		puts "#{test_file_name} OK"
	else
		puts "\n\n#{test_file_name} FAILED\n"
		puts "### got:"
		results.each_line { |l| puts '# ' + l }
		puts "\n"
		puts "### needed:"
		if goal
			goal.each_line { |l| puts '# ' + l }
			puts "\n\n"
		else
			puts "error: #{test_file_name} contains no '### TEST RESULT' line...\n\n"
		end

		if quick_fix
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
				results.each_line { |l| new_test += "# #{l}" }
				f = File.new(test_file_name, 'w')
				f.write(new_test)
				f.close
				puts "fixed..."
			else
				puts "not fixed..."
			end
		end
	end
end



