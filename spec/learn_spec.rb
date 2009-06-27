require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "learn (user defined commands)" do
  it "should work for commands without arguments"
  it "should work for commands with 1 argument"
  it "should work for commands with 2 mixed arguments"
  it "should work for commands with 3 mixed arguments"
  it "should work with return values"
  it "should produce an error when the number of arguments does not match"
  it "should produce an error when the scope is omitted"
  it "should allow recursion over one function"
  it "should allow recursion over two functions"
end