require File.dirname(__FILE__) + '/spec_helper.rb'
$i = Interpreter.instance

describe "expression" do
  it "should work as expected with basic operations (==, !=, >, <, >=, <=)" do
    $i.should_run_clean <<-EOS
      assert 1 == 1
      assert 1 != 2
      assert 2 > 1
      assert 1 < 2

      assert 1 >= 1
      assert 2 >= 1
      assert 1 <= 1
      assert 1 <= 2
    EOS
  end

  it "should raise an error when insufficient values are provided" do
    $i.run("assert 1 <").errors?.should be_true
    $i.run("assert ==").errors?.should be_true
  end
end
