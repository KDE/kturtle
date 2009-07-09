require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "boolean operator" do
  it "should work as expected with basic operations (and, or, not)" do
    $i.should_run_clean <<-EOS
      assert true and true
      assert true or true
      assert not false
    EOS
  end

  it "should raise an error when insufficient values are provided" do
    $i.run("assert true and").errors?.should be_true
    $i.run("assert and true").errors?.should be_true
    $i.run("assert or").errors?.should be_true
    $i.run("assert not").errors?.should be_true
  end
end
