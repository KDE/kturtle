#  Copyright (C) 2009 by Cies Breijs
#  Copyright (C) 2009 by Niels Slot
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

describe "for loop" do
  it "should work as expected for ascending positive values" do
    $i.should_run_clean <<-EOS
      $y = 1

      for $x = 1 to 10 {
	assert $x == $y
        $y = $y + 1
      }
      assert $y == 11
    EOS
  end

  it "should work as expected for ascending negative values" do
    $i.should_run_clean <<-EOS
      $y = -10

      for $x = -10 to -1 {
	assert $x == $y
	$y = $y + 1
      }
      assert $y == 0
    EOS
  end

  it "should work as expected for descending positive values" do
    $i.should_run_clean <<-EOS
      $y = 10

      for $x = 10 to 1 step -1 {
	assert $x == $y
	$y = $y - 1
      }
      assert $y == 0
    EOS
  end

  it "should work as expected for descending negative values" do
    $i.should_run_clean <<-EOS
      $y = -1

      for $x = -1 to -10 step -1 {
	assert $x == $y
	$y = $y - 1
      }
      assert $y == -11
    EOS
  end

  it "should work as expected with step > 1" do
    $i.should_run_clean <<-EOS
      $y = 0

      for $x = 0 to 10 step 2 {
	assert $x == $y
	$y = $y + 2
      }
      assert $y == 12
    EOS
  end

  it "should work as expected with 0 > step > 1" do
    $i.should_run_clean <<-EOS
      $y = 1

      for $x = 1 to 10 step 0.5 {
	assert $x == $y
	$y = $y + 0.5
      }
      assert $y == 10.5
    EOS
  end

  it "should work as expected with -1 > step > 0" do
    $i.should_run_clean <<-EOS
      $y = 10

      for $x = 10 to 1 step -0.5 {
	assert $x == $y
	$y = $y - 0.5
      }
      assert $y == 0.5
    EOS
  end

  it "should work as expected with step < -1" do
    $i.should_run_clean <<-EOS
      $y = 10

      for $x = 10 to 0 step -2 {
	assert $x == $y
	$y = $y - 2
      }
      assert $y == -2
    EOS
  end

  it "should work as expected when no execution required" do
    $i.should_run_clean <<-EOS
      for $x = 2 to 1 {
	assert false
      }
    EOS
  end
end
