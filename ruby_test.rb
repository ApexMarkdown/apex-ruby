#! /usr/bin/env ruby
# frozen_string_literal: true

require 'apex'

text = IO.read(File.join('..', 'apex', 'tests', 'fixtures', 'comprehensive_test.md'))

doc = Apex::Document.new(text, mode: :gfm, enable_tables: true)
puts doc.to_html
