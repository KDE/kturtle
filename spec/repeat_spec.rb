#  SPDX-FileCopyrightText: 2009 Cies Breijs
#
#  SPDX-License-Identifier: GPL-2.0-or-later

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
    # TODO decide: do we really want this? or do we like the current behavior in which unassigned 
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
