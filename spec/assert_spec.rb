#  SPDX-FileCopyrightText: 2009 Cies Breijs
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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