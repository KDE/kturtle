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

require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "string" do
  it "should work as expected with simple strings" do
    $i.should_run_clean <<-EOS
      $x = "test"
      $y = "kturtle"
      assert $x == "test"
      assert $y == "kturtle"

      $cies = "Cies"
      $author = "Cies"
      assert $cies == $author
    EOS
  end

  it "should work as expected with unicode characters"

  it "should work as expected with escaped quotes" do
    $i.should_run_clean <<-EOS
      $x = "\\"quote"
      $y = "\\"quotes\\""

      # We currently have no way to validate if the string 
      #  was correctly parsed by KTurtle. e.g. printing a 
      #	 string with quotes also prints the slashes.

      assert false
    EOS
  end

  it "should allow string concatenation" do
    $i.should_run_clean <<-EOS
      $x = "K"
      $y = "Turtle"
      $z = $x + $y
      assert $z == "KTurtle"

      $first = "Cies"
      $last = "Breijs"
      $name = $first + " " + $last
      assert $name == "Cies Breijs"
    EOS
  end

  it "should produce an error when not properly terminated" do
    $i.run("\"test").errors?.should be_true
  end
end