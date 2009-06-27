require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "string" do
  it "should work as expected with simple strings"
  it "should work as expected with unicode characters"
  it "should work as expected with escaped quotes"
  it "should allow string concatenation"
  it "should produce an error when not properly terminated"
end