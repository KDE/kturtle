#  SPDX-FileCopyrightText: 2009 Cies Breijs
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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
