require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "for loop" do
  it "should work as expected for ascending positive values"
  it "should work as expected for ascending negative values"
  it "should work as expected for descending positive values"
  it "should work as expected for descending negative values"
  it "should work as expected with step > 1"
  it "should work as expected with 0 > step > 1"
  it "should work as expected with -1 > step > 0"
  it "should work as expected with step < -1"
  it "should work as expected when no execution required"
end