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

describe "scopes" do
  it "should allow empty scopes" do
    true.should_not be_true
    # empty scopes currently hang the interpreter
    # so do scopes without a flow/learn statement
    # TODO fix this. (without the fix the other tests are useless)
  end

  it "should error on unbalanced scopes"
  it "should properly localize variables"
end
