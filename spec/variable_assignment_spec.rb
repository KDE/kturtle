require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "variable assignment" do
  it "should be able to contain numbers"
  it "should be able to contain strings"
  it "should be able to contain boolean values"
  it "should be able to be empty"
  it "should be usable mathmatical operators"
  it "should be usable in expressions"
  it "should change type automatically"
  it "should accept names according to the convention"
  it "should produce an error when not named according to the convention"
end