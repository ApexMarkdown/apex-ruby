#include "ruby.h"
#include "apex/apex.h"
#include <strings.h>

static VALUE mApex;
static VALUE mApexNative;

/* Map Ruby mode symbol/string to apex_mode_t. */
static apex_mode_t
mode_from_ruby(VALUE mode_val)
{
  if (NIL_P(mode_val)) return APEX_MODE_UNIFIED;

  VALUE str = rb_funcall(mode_val, rb_intern("to_s"), 0);
  const char *cstr = StringValueCStr(str);

  if (!strcasecmp(cstr, "commonmark") || !strcasecmp(cstr, "cmark"))
    return APEX_MODE_COMMONMARK;
  if (!strcasecmp(cstr, "gfm") || !strcasecmp(cstr, "github"))
    return APEX_MODE_GFM;
  if (!strcasecmp(cstr, "multimarkdown") || !strcasecmp(cstr, "mmd"))
    return APEX_MODE_MULTIMARKDOWN;
  if (!strcasecmp(cstr, "kramdown"))
    return APEX_MODE_KRAMDOWN;
  if (!strcasecmp(cstr, "unified"))
    return APEX_MODE_UNIFIED;

  return APEX_MODE_UNIFIED;
}

static void
set_bool(VALUE h, const char *key, bool *field)
{
  ID id = rb_intern(key);
  VALUE k = ID2SYM(id);
  VALUE v = rb_funcall(h, rb_intern("key?"), 1, k);
  if (RTEST(v)) {
    VALUE val = rb_hash_aref(h, k);
    *field = RTEST(val);
  }
}

static void
set_int(VALUE h, const char *key, int *field)
{
  ID id = rb_intern(key);
  VALUE k = ID2SYM(id);
  VALUE v = rb_funcall(h, rb_intern("key?"), 1, k);
  if (RTEST(v)) {
    VALUE val = rb_hash_aref(h, k);
    if (!NIL_P(val)) *field = NUM2INT(val);
  }
}

static void
set_cstr(VALUE h, const char *key, const char **field)
{
  ID id = rb_intern(key);
  VALUE k = ID2SYM(id);
  VALUE v = rb_funcall(h, rb_intern("key?"), 1, k);
  if (!RTEST(v)) return;

  VALUE val = rb_hash_aref(h, k);
  if (NIL_P(val)) {
    *field = NULL;
  } else {
    val = StringValue(val);
    *field = StringValueCStr(val);
  }
}

/* Build apex_options from a Ruby Hash. */
static apex_options
options_from_hash(VALUE h)
{
  apex_options o = apex_options_default();

  if (NIL_P(h) || TYPE(h) != T_HASH) return o;

  VALUE mode_val = rb_hash_aref(h, ID2SYM(rb_intern("mode")));
  o.mode = mode_from_ruby(mode_val);

  /* Feature flags */
  set_bool(h, "enable_plugins",     &o.enable_plugins);
  set_bool(h, "enable_tables",      &o.enable_tables);
  set_bool(h, "enable_footnotes",   &o.enable_footnotes);
  set_bool(h, "enable_definition_lists", &o.enable_definition_lists);
  set_bool(h, "enable_smart_typography", &o.enable_smart_typography);
  set_bool(h, "enable_math",        &o.enable_math);
  set_bool(h, "enable_wiki_links",  &o.enable_wiki_links);
  set_bool(h, "enable_task_lists",  &o.enable_task_lists);
  set_bool(h, "enable_attributes",  &o.enable_attributes);
  set_bool(h, "enable_callouts",    &o.enable_callouts);
  set_bool(h, "enable_divs",        &o.enable_divs);
  set_bool(h, "enable_spans",       &o.enable_spans);

  /* Metadata handling */
  set_bool(h, "strip_metadata",            &o.strip_metadata);
  set_bool(h, "enable_metadata_variables", &o.enable_metadata_variables);
  set_bool(h, "enable_metadata_transforms",&o.enable_metadata_transforms);

  /* File inclusion */
  set_bool(h, "enable_file_includes", &o.enable_file_includes);
  set_int (h, "max_include_depth",    &o.max_include_depth);
  set_cstr(h, "base_directory",       &o.base_directory);

  /* Output options */
  set_bool(h, "unsafe",          &o.unsafe);
  set_bool(h, "validate_utf8",   &o.validate_utf8);
  set_bool(h, "github_pre_lang", &o.github_pre_lang);
  set_bool(h, "standalone",      &o.standalone);
  set_bool(h, "pretty",          &o.pretty);
  set_cstr(h, "document_title",  &o.document_title);

  /* Line break handling */
  set_bool(h, "hardbreaks", &o.hardbreaks);
  set_bool(h, "nobreaks",   &o.nobreaks);

  /* Header ID generation */
  set_bool(h, "generate_header_ids", &o.generate_header_ids);
  set_bool(h, "header_anchors",      &o.header_anchors);
  set_int (h, "id_format",           &o.id_format);

  /* Table options */
  set_bool(h, "relaxed_tables",     &o.relaxed_tables);
  set_int (h, "caption_position",   &o.caption_position);
  set_bool(h, "per_cell_alignment", &o.per_cell_alignment);

  /* List options */
  set_bool(h, "allow_mixed_list_markers", &o.allow_mixed_list_markers);
  set_bool(h, "allow_alpha_lists",        &o.allow_alpha_lists);

  /* Superscript and subscript */
  set_bool(h, "enable_sup_sub",      &o.enable_sup_sub);

  /* Strikethrough */
  set_bool(h, "enable_strikethrough",&o.enable_strikethrough);

  /* Autolink options */
  set_bool(h, "enable_autolink",  &o.enable_autolink);
  set_bool(h, "obfuscate_emails", &o.obfuscate_emails);

  /* Image options */
  set_bool(h, "enable_image_captions", &o.enable_image_captions);
  set_bool(h, "title_captions_only",   &o.title_captions_only);

  /* Wiki link options */
  set_int (h, "wikilink_space",     &o.wikilink_space);
  set_cstr(h, "wikilink_extension", &o.wikilink_extension);
  set_bool(h, "wikilink_sanitize",  &o.wikilink_sanitize);

  /* ARIA accessibility options */
  set_bool(h, "enable_aria", &o.enable_aria);

  /* Emoji options */
  set_bool(h, "enable_emoji_autocorrect", &o.enable_emoji_autocorrect);

  /* Syntax highlighting options */
  set_cstr(h, "code_highlighter",    &o.code_highlighter);
  set_bool(h, "code_line_numbers",   &o.code_line_numbers);
  set_bool(h, "highlight_language_only", &o.highlight_language_only);

  /* Marked / integration-specific options */
  set_bool(h, "enable_widont",           &o.enable_widont);
  set_bool(h, "code_is_poetry",          &o.code_is_poetry);
  set_bool(h, "enable_markdown_in_html", &o.enable_markdown_in_html);
  set_bool(h, "random_footnote_ids",     &o.random_footnote_ids);
  set_bool(h, "enable_hashtags",         &o.enable_hashtags);
  set_bool(h, "style_hashtags",          &o.style_hashtags);
  set_bool(h, "proofreader_mode",        &o.proofreader_mode);
  set_bool(h, "hr_page_break",           &o.hr_page_break);
  set_bool(h, "title_from_h1",           &o.title_from_h1);
  set_bool(h, "page_break_before_footnotes", &o.page_break_before_footnotes);

  return o;
}

/*
 * call-seq:
 *   Apex::Native.markdown_to_html(text, options = {}) -> String
 *
 * Render Markdown +text+ to HTML using the Apex C library.
 */
static VALUE
rb_apex_markdown_to_html(int argc, VALUE *argv, VALUE self)
{
  VALUE text, opts_hash;
  rb_scan_args(argc, argv, "11", &text, &opts_hash);

  Check_Type(text, T_STRING);
  if (NIL_P(opts_hash)) opts_hash = rb_hash_new();

  const char *c_text = RSTRING_PTR(text);
  size_t len = (size_t)RSTRING_LEN(text);

  apex_options opts = options_from_hash(opts_hash);

  char *out = apex_markdown_to_html(c_text, len, &opts);
  if (!out) return Qnil;

  VALUE rb_out = rb_utf8_str_new_cstr(out);
  apex_free_string(out);
  return rb_out;
}

void
Init_apex_ext(void)
{
  mApex       = rb_define_module("Apex");
  mApexNative = rb_define_module_under(mApex, "Native");

  rb_define_singleton_method(mApexNative, "markdown_to_html",
                             rb_apex_markdown_to_html, -1);
}

