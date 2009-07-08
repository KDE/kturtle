require 'rubygems'
require 'singleton'

begin
  require 'Qt'
rescue LoadError
  begin
    require 'rbus'
  rescue LoadError
    puts "Couldn't load either rdbus or Qt, please install one, quiting now.."
    exit
  end
end

class Interpreter
  include Singleton

  def connect
    @pid = ENV['KTURTLE_INTERPRETER_DBUS_PID'] || IO.readlines("pid")[0].to_i
    puts "Opening DBus connection with KTurtle (pid: #{@pid})..."

    # Use Qt DBus if it's found, otherwise fall back to rbus
    if Object.const_defined?(:Qt)
      @interpreter = Qt::DBusInterface.new("org.kde.kturtle-#{@pid}", '/Interpreter', 'org.kde.kturtle.Interpreter')
    else
      @interpreter = RBus.session_bus.get_object("org.kde.kturtle-#{@pid}", '/Interpreter')
      @interpreter.interface!('org.kde.kturtle.Interpreter')
    end
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