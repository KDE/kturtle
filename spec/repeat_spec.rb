#  Copyright (C) 2009 by Cies Breijs
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

describe "repeat loop" do
  it "should work for positive natural numbers" do
    $i.should_run_clean <<-EOS
      $x1 = 0
      repeat 1 {
        $x1 = $x1 + 1
      }
      assert $x1 == 1

      $x2 = 0
      repeat 2 {
        $x2 = $x2 + 1
      }
      assert $x2 == 2

      $x3 = 0
      repeat 3 {
        $x3 = $x3 + 1
      }
      assert $x3 == 3
    EOS
  end

  it "should work for positive decimal numbers" do
    $i.should_run_clean <<-EOS
      $x1 = 0
      repeat 1.1 {
        $x1 = $x1 + 1
      }
      assert $x1 == 1

      $x2 = 0
      repeat 1.5 {
        $x2 = $x2 + 1
      }
      assert $x2 == 2

      $x3 = 0
      repeat 2.9 {
        $x3 = $x3 + 1
      }
      assert $x3 == 3
    EOS
  end

  it "should not execute the scope when argument is < 0.5" do
    $i.should_run_clean <<-EOS
      repeat 0.49 {
        assert false
      }
      repeat 0.1 {
        assert false
      }
    EOS
  end

  it "should not execute the scope when argument is zero" do
    $i.should_run_clean <<-EOS
      repeat 0 {
        assert false
      }
    EOS
  end

  it "should not execute the scope when argument is empty" do
    # TODO decide: do we really want this? or do we like the current behaivior in which unassigned 
    $i.should_run_clean <<-EOS
      repeat $x {
        assert false
      }
    EOS
  end

  it "should not execute the scope when argument is negative" do
    $i.should_run_clean <<-EOS
      repeat -.1 {
        assert false
      }
      repeat -1 {
        assert false
      }
      repeat -10 {
        assert false
      }
    EOS
  end

  it "should produce an error when a string is given" do
    $i.run('repeat "123" {}').errors?.should be_true
  end

  it "should produce an error when a boolean is given" do
    $i.run('repeat true {}').errors?.should be_true
  end

  it "should produce an error when argument omitted" do
    $i.run('repeat {}').errors?.should be_true
  end

  it "should produce an error when scope is omitted" do
    $i.run('repeat 3').errors?.should be_true
  end
end