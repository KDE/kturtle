#  SPDX-FileCopyrightText: 2009 Cies Breijs
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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