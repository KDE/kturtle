#  SPDX-FileCopyrightText: 2009 Cies Breijs
#  SPDX-FileCopyrightText: 2009 Niels Slot
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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