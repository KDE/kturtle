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

describe "if (and else) statement" do

  it "should work with one clause" do
    $i.should_run_clean <<-EOS
      if true {
        assert true
      }
      if false {
        assert false  # not executed
      }
    EOS
  end

  it "should work with two clauses" do
    $i.should_run_clean <<-EOS
      if true {
        assert true
      } else {
        assert false  # not executed
      }
      if false {
        assert false  # not executed
      } else {
        assert true
      }
    EOS
  end

  it "should be flexible" do
    $i.should_run_clean <<-EOS
      if
      if true
      if false
      if true {}
      if false {}
    EOS
  end

  it "should not allow scopeless oneliners" do
    $i.should_run_clean <<-EOS
      $x = 0
      if false
        $x = 1  # should be executed, it's outside the if clause
      assert $x == 1
    EOS
  end

  it "should work when nested" do
    $i.should_run_clean <<-EOS
      $x = 0
      if true {
        if true {
          if true {
            if false {
              assert false
            } else {
              $x = 1
            }
          }
        }
      }
      assert $x == 1
    EOS
  end
end