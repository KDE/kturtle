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

describe "number" do
  it "should accept negative numbers" do
    $i.should_run_clean <<-EOS
      $x = -1
      $x = -1000
    EOS
  end

  it "should accept numbers with a decimal point" do
    $i.should_run_clean <<-EOS
      $x = 1.23
      $x = -1.23
    EOS
  end

  it "should automatically remove floating point" do
    $i.should_run_clean <<-EOS
      assert 1.00000 == 1
      assert -123.0 == -123
    EOS
  end
end