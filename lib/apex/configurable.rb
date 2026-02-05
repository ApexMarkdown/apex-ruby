# frozen_string_literal: true

module Apex
  module Utils
    # Mixin providing a simple registry of configurable extensions,
    # modeled after +Kramdown::Utils::Configurable+.
    #
    # @example
    #   module Apex
    #     extend Apex::Utils::Configurable
    #
    #     configurable :syntax_highlighter
    #   end
    #
    #   Apex.add_syntax_highlighter(:rouge, ->(code, lang, opts) { ... })
    #   Apex.syntax_highlighter(:rouge) # => registered callable
    module Configurable
      # Declare a configurable extension point.
      #
      # This defines three singleton methods on the receiver:
      #
      # * +configurables+ – a Hash-of-Hashes registry.
      # * +name(ext_name)+ – retrieve the registered data for +ext_name+.
      # * +add_name(ext_name, data = nil, &block)+ – register +ext_name+.
      #
      # @param name [Symbol, String] the logical name of the configurable
      # @return [void]
      def configurable(name)
        unless respond_to?(:configurables)
          singleton_class.send(:define_method, :configurables) do
            @_configurables ||= Hash.new { |h, k| h[k] = {} }
          end
        end

        singleton_class.send(:define_method, name) do |ext_name|
          configurables[name][ext_name]
        end

        singleton_class.send(:define_method, :"add_#{name}") do |ext_name, data = nil, &block|
          configurables[name][ext_name] = data || block
        end
      end
    end
  end
end

