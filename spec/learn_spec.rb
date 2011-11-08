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

describe "learn (user defined commands)" do
  it "should work for commands without arguments" do
    $i.should_run_clean <<-EOS
      learn user_def {
        assert true
      }
      user_def
    EOS
  end

  it "should work for commands with 1 argument" do
    $i.should_run_clean <<-EOS
      learn user_def $x {
	assert $x == 1
      }
      user_def 1
    EOS
  end

  it "should work for commands with 2 mixed arguments" do
    $i.should_run_clean <<-EOS
      learn user_def $x, $y {
	assert $x == "kturtle"
	assert $y == 10
      }
      user_def "kturtle", 10
    EOS
  end

  it "should work for commands with 3 mixed arguments" do
    $i.should_run_clean <<-EOS
      learn user_def $x, $y, $z {
	assert $x
	assert $y == "kturtle"
	assert $z == 10
      }
      user_def true, "kturtle", 10
    EOS
  end

  it "should work with return values" do
    $i.should_run_clean <<-EOS
      learn user_def {
	return true
      }
      assert user_def

      learn user_def2 {
	return "kturtle"
      }
      assert "kturtle" == user_def2
    EOS
  end

  it "should produce an error when the number of arguments does not match" do
    $i.run <<-EOS
      learn user_def $x, $y, $z {

      }
      user_def 1
    EOS
    $i.errors?.should be_true
  end

  it "should produce an error when the scope is omitted" do
    $i.run <<-EOS
      learn user_def $x

      user_def
    EOS
    $i.errors?.should be_true
  end

  it "should allow recursion over one function"
  it "should allow recursion over two functions"
end
