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

describe "variable assignment" do
  it "should be able to contain numbers" do
    $i.should_run_clean <<-EOS
      $x = 1
      $y = 1.2
      assert $x == 1
      assert $y == 1.2
    EOS
  end

  it "should be able to contain strings" do
    $i.should_run_clean <<-EOS
      $x = "string"
      $y = "kturtle"
      assert $x == "string"
      assert $y == "kturtle
    EOS
  end

  it "should be able to contain boolean values" do
    $i.should_run_clean <<-EOS
      $x = true
      $y = false
      assert $x == true
      assert $y == false
    EOS
  end

  it "should be able to be empty"
  it "should be usable mathmatical operators"
  it "should be usable in expressions"
  it "should change type automatically"
  it "should accept names according to the convention"
  it "should produce an error when not named according to the convention"
end