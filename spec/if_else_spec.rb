#  SPDX-FileCopyrightText: 2009 Cies Breijs
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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