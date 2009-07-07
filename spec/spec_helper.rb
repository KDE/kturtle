require 'rubygems'
require 'rbus'
require 'singleton'

class Interpreter
  include Singleton

  def connect
    @pid = ENV['KTURTLE_INTERPRETER_DBUS_PID'] || IO.readlines("pid")[0].to_i
    puts "Opening DBus connection with KTurtle (pid: #{@pid})..."
    @interpreter = RBus.session_bus.get_object("org.kde.kturtle-#{@pid}", '/Interpreter')
    @interpreter.interface!('org.kde.kturtle.Interpreter')
  end

  def load(code); connect unless @pid; @interpreter.initialize code; nil; end
  def interpret;  @interpreter.interpret; nil; end
  def errors?;    @interpreter.encounteredErrors; end
  def errors;     @interpreter.getErrorStrings; end
  def state;      [:uninitialized, :initialized, :parsing, :executing, :finished, :aborted][@interpreter.state]; end
  def inspect;    "#<Interpreter pid:#{@pid}>"; end

  def run(code)
    load code
    while not [:finished, :aborted].include? state
      interpret
    end
    self  # return self for easy method stacking
  end

  def should_run_clean(code)
    run(code)
    errors?.should == false
    p errors if errors?
  end
end