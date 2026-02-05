# frozen_string_literal: true

require_relative "configurable"

module Apex
  # High-level document API for Apex Markdown.
  #
  # The primary entry point is {.markdown_to_html}, which mirrors the
  # style of +Kramdown::Document.new(text, options).to_html+ but uses
  # a single convenient class method:
  #
  #   html = Apex::Document.markdown_to_html(text, mode: :unified)
  #
  # You can also use the instance-level API if you prefer object
  # instances that you can reuse:
  #
  #   doc = Apex::Document.new(text, mode: :gfm, enable_tables: true)
  #   html = doc.to_html
  class Document
    # Render Markdown to HTML using Apex.
    #
    # @param source [#to_s] the Markdown source text
    # @param mode [Symbol, String] processing mode (:unified by default).
    #   Accepted values:
    #   * +:unified+
    #   * +:gfm+, +:github+
    #   * +:multimarkdown+, +:mmd+
    #   * +:commonmark+, +:cmark+
    #   * +:kramdown+
    # @param options [Hash] additional Apex options, mapped directly to
    #   the underlying +apex_options+ struct (see C header for details).
    # @return [String] rendered HTML
    def self.markdown_to_html(source, mode: :unified, **options)
      opts = { mode: mode }.merge(options)
      Apex::Native.markdown_to_html(String(source), opts)
    end

    # @return [String] the original Markdown source
    attr_reader :source

    # @return [Symbol, String] the selected Apex mode
    attr_reader :mode

    # @return [Hash] options passed on to the native Apex renderer
    attr_reader :options

    # Create a new document instance.
    #
    # @param source [#to_s] the Markdown source text
    # @param mode [Symbol, String] processing mode, defaults to +:unified+
    # @param options [Hash] additional Apex options
    def initialize(source, mode: :unified, **options)
      @source  = String(source)
      @mode    = mode
      @options = options
    end

    # Render this document to HTML.
    #
    # @return [String] rendered HTML
    def to_html
      self.class.markdown_to_html(@source, mode: @mode, **@options)
    end
  end
end

