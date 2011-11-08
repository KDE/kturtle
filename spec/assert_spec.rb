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

describe "assert statement" do

  it "should pass when evaluating true" do
    $i.should_run_clean 'assert true'
  end

  it "should fail when evaluating false" do
    $i.run('assert false').errors?.should be_true
  end

  it "should not take anything but a boolean argument" do
    $i.run('assert 123').errors?.should be_true
    $i.run('assert 1.23').errors?.should be_true
    $i.run('assert "qwe"').errors?.should be_true
  end

  it "should take exactly one argument" do
    $i.run('assert').errors?.should be_true
    $i.run('assert true, true').errors?.should be_true
  end

end