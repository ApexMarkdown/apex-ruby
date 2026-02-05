require "minitest/autorun"
require "apex"

class ApexDocumentTest < Minitest::Test
  # Basic smoke test to ensure the native extension loads and we can
  # render a simple Markdown string.
  def test_markdown_to_html_unified_default
    html = Apex::Document.markdown_to_html("**Hello**")
    assert_kind_of String, html
    assert_includes html, "Hello"
  end

  def test_markdown_to_html_gfm_mode
    html = Apex::Document.markdown_to_html("* item", mode: :gfm, enable_task_lists: true)
    assert_kind_of String, html
    assert_includes html, "item"
  end

  def test_instance_api_delegates_to_class
    doc  = Apex::Document.new("# Title", mode: :kramdown, generate_header_ids: true)
    html = doc.to_html
    assert_kind_of String, html
    assert_includes html, "Title"
  end
end

