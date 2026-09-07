require "minitest/autorun"
require "apex"
require "tempfile"

class ApexDocumentTest < Minitest::Test
  def test_markdown_to_html_unified_default
    html = Apex::Document.markdown_to_html("**Hello**")
    assert_kind_of String, html
    assert_includes html, "Hello"
  end

  def test_native_reports_apex_c_version
    assert_match(/\A\d+\.\d+\.\d+\z/, Apex::Native.version)
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

  def test_yaml_front_matter_stripped_in_unified
    md = <<~MD
      ---
      title: From YAML
      ---

      Hello body
    MD
    html = Apex::Document.markdown_to_html(md, mode: :unified)
    assert_includes html, "Hello body"
    refute_includes html, "title: From YAML"
    refute_match(/<hr/i, html)
  end

  def test_yaml_front_matter_not_stripped_in_gfm
    md = <<~MD
      ---
      title: From YAML
      ---

      Hello body
    MD
    html = Apex::Document.markdown_to_html(md, mode: :gfm)
    assert_includes html, "Hello body"
    # GFM does not extract metadata; opening --- becomes <hr>
    assert_match(/<hr/i, html)
  end

  def test_definition_list_then_fence
    md = <<~MD
      Term
      : Def

      ```
      code
      ```
    MD
    html = Apex::Document.markdown_to_html(md, mode: :unified)
    assert_includes html, "<dl>"
    assert_includes html, "<pre"
    assert_includes html, "<code"
    refute_includes html, "</dl>\n```"
  end

  def test_textindex_indices_enabled_by_unified_defaults
    html = Apex::Document.markdown_to_html("firmware{^}", mode: :unified)
    assert_includes html, "class=\"index\""
  end

  def test_concordance_option
    Tempfile.create(["apex-conc", ".tsv"]) do |f|
      f.write("widget\tgadget\n")
      f.flush
      html = Apex::Document.markdown_to_html(
        "A widget appears.",
        mode: :unified,
        concordance: f.path
      )
      assert_includes html, "gadget"
      assert_includes html, "index-return"
    end
  end
end
