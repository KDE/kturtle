#  SPDX-FileCopyrightText: 2009 Cies Breijs
#  SPDX-FileCopyrightText: 2009 Niels Slot
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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