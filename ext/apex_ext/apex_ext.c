#include "ruby.h"
#include "apex/apex.h"
#include "extensions/metadata.h"
#include <stdlib.h>
#include <string.h>
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
  if (!strcasecmp(cstr, "quarto"))
    return APEX_MODE_QUARTO;

  return APEX_MODE_UNIFIED;
}

static int
hash_has_key(VALUE h, const char *key)
{
  ID id = rb_intern(key);
  VALUE k = ID2SYM(id);
  VALUE v = rb_funcall(h, rb_intern("key?"), 1, k);
  return RTEST(v);
}

static void
set_bool(VALUE h, const char *key, bool *field)
{
  if (!hash_has_key(h, key)) return;
  VALUE val = rb_hash_aref(h, ID2SYM(rb_intern(key)));
  *field = RTEST(val);
}

static void
set_int(VALUE h, const char *key, int *field)
{
  if (!hash_has_key(h, key)) return;
  VALUE val = rb_hash_aref(h, ID2SYM(rb_intern(key)));
  if (!NIL_P(val)) *field = NUM2INT(val);
}

static void
set_cstr(VALUE h, const char *key, const char **field)
{
  if (!hash_has_key(h, key)) return;
  VALUE val = rb_hash_aref(h, ID2SYM(rb_intern(key)));
  if (NIL_P(val)) {
    *field = NULL;
  } else {
    val = StringValue(val);
    *field = StringValueCStr(val);
  }
}

/* Build a NULL-terminated strdup'd C string array from a Ruby Array or String.
 * Caller must free with free_cstr_array(). */
static char **
cstr_array_from_ruby(VALUE h, const char *key)
{
  if (!hash_has_key(h, key)) return NULL;

  VALUE val = rb_hash_aref(h, ID2SYM(rb_intern(key)));
  if (NIL_P(val)) return NULL;

  if (TYPE(val) == T_STRING) {
    char **arr = malloc(2 * sizeof(char *));
    if (!arr) return NULL;
    arr[0] = strdup(StringValueCStr(val));
    arr[1] = NULL;
    if (!arr[0]) { free(arr); return NULL; }
    return arr;
  }

  if (TYPE(val) != T_ARRAY) return NULL;

  long n = RARRAY_LEN(val);
  char **arr = malloc((size_t)(n + 1) * sizeof(char *));
  if (!arr) return NULL;

  long count = 0;
  for (long i = 0; i < n; i++) {
    VALUE item = rb_ary_entry(val, i);
    if (NIL_P(item)) continue;
    item = StringValue(item);
    arr[count] = strdup(StringValueCStr(item));
    if (!arr[count]) {
      for (long j = 0; j < count; j++) free(arr[j]);
      free(arr);
      return NULL;
    }
    count++;
  }
  arr[count] = NULL;
  if (count == 0) { free(arr); return NULL; }
  return arr;
}

static void
free_cstr_array(char **arr)
{
  if (!arr) return;
  for (size_t i = 0; arr[i]; i++) free(arr[i]);
  free(arr);
}

/* Apply kwargs that override mode defaults / document metadata. */
static void
apply_kwargs(VALUE h, apex_options *o, char ***bib_out, char ***conc_out)
{
  /* Feature flags */
  set_bool(h, "enable_plugins",     &o->enable_plugins);
  set_bool(h, "enable_tables",      &o->enable_tables);
  set_bool(h, "enable_footnotes",   &o->enable_footnotes);
  set_bool(h, "enable_definition_lists", &o->enable_definition_lists);
  set_bool(h, "enable_smart_typography", &o->enable_smart_typography);
  set_bool(h, "enable_math",        &o->enable_math);
  set_bool(h, "enable_critic_markup", &o->enable_critic_markup);
  set_bool(h, "enable_wiki_links",  &o->enable_wiki_links);
  set_bool(h, "enable_task_lists",  &o->enable_task_lists);
  set_bool(h, "enable_attributes",  &o->enable_attributes);
  set_bool(h, "enable_callouts",    &o->enable_callouts);
  set_bool(h, "enable_py_callouts", &o->enable_py_callouts);
  set_bool(h, "enable_quarto_callouts", &o->enable_quarto_callouts);
  set_bool(h, "enable_marked_extensions", &o->enable_marked_extensions);
  set_bool(h, "enable_divs",        &o->enable_divs);
  set_bool(h, "enable_spans",       &o->enable_spans);
  set_bool(h, "enable_grid_tables", &o->enable_grid_tables);

  set_int(h, "critic_mode", &o->critic_mode);

  /* Metadata handling */
  set_bool(h, "strip_metadata",            &o->strip_metadata);
  set_bool(h, "enable_metadata_variables", &o->enable_metadata_variables);
  set_bool(h, "enable_metadata_transforms",&o->enable_metadata_transforms);

  /* File inclusion */
  set_bool(h, "enable_file_includes", &o->enable_file_includes);
  set_int (h, "max_include_depth",    &o->max_include_depth);
  set_cstr(h, "base_directory",       &o->base_directory);

  /* Output options */
  set_bool(h, "unsafe",          &o->unsafe);
  set_bool(h, "validate_utf8",   &o->validate_utf8);
  set_bool(h, "github_pre_lang", &o->github_pre_lang);
  set_bool(h, "standalone",      &o->standalone);
  set_bool(h, "pretty",          &o->pretty);
  set_bool(h, "xhtml",           &o->xhtml);
  set_bool(h, "strict_xhtml",    &o->strict_xhtml);
  set_cstr(h, "document_title",  &o->document_title);

  /* Line break handling */
  set_bool(h, "hardbreaks", &o->hardbreaks);
  set_bool(h, "nobreaks",   &o->nobreaks);

  /* Header ID generation */
  set_bool(h, "generate_header_ids", &o->generate_header_ids);
  set_bool(h, "header_anchors",      &o->header_anchors);
  set_int (h, "id_format",           &o->id_format);
  set_int (h, "toc_min",             &o->toc_min);
  set_int (h, "toc_max",             &o->toc_max);

  /* Table options */
  set_bool(h, "relaxed_tables",     &o->relaxed_tables);
  set_int (h, "caption_position",   &o->caption_position);
  set_bool(h, "per_cell_alignment", &o->per_cell_alignment);

  /* List options */
  set_bool(h, "allow_mixed_list_markers", &o->allow_mixed_list_markers);
  set_bool(h, "allow_alpha_lists",        &o->allow_alpha_lists);

  /* Superscript and subscript */
  set_bool(h, "enable_sup_sub",      &o->enable_sup_sub);

  /* Strikethrough */
  set_bool(h, "enable_strikethrough",&o->enable_strikethrough);

  /* Autolink options */
  set_bool(h, "enable_autolink",  &o->enable_autolink);
  set_bool(h, "obfuscate_emails", &o->obfuscate_emails);

  /* Image options */
  set_bool(h, "embed_images",          &o->embed_images);
  set_bool(h, "enable_image_captions", &o->enable_image_captions);
  set_bool(h, "title_captions_only",   &o->title_captions_only);

  /* Citations */
  set_bool(h, "enable_citations",       &o->enable_citations);
  set_cstr(h, "csl_file",               &o->csl_file);
  set_bool(h, "suppress_bibliography",  &o->suppress_bibliography);
  set_bool(h, "link_citations",         &o->link_citations);
  set_bool(h, "show_tooltips",          &o->show_tooltips);
  set_cstr(h, "nocite",                 &o->nocite);

  char **bib = cstr_array_from_ruby(h, "bibliography_files");
  if (!bib) bib = cstr_array_from_ruby(h, "bibliography");
  if (bib) {
    *bib_out = bib;
    o->bibliography_files = bib;
    o->enable_citations = true;
  }

  /* Indices / TextIndex / concordance */
  set_bool(h, "enable_indices",               &o->enable_indices);
  set_bool(h, "enable_mmark_index_syntax",    &o->enable_mmark_index_syntax);
  set_bool(h, "enable_textindex_syntax",      &o->enable_textindex_syntax);
  set_bool(h, "enable_leanpub_index_syntax",  &o->enable_leanpub_index_syntax);
  set_bool(h, "suppress_index",               &o->suppress_index);
  set_bool(h, "group_index_by_letter",        &o->group_index_by_letter);

  char **conc = cstr_array_from_ruby(h, "concordance_files");
  if (!conc) conc = cstr_array_from_ruby(h, "concordance");
  if (conc) {
    *conc_out = conc;
    o->concordance_files = conc;
    o->enable_indices = true;
    o->enable_textindex_syntax = true;
  }

  /* Wiki link options */
  set_int (h, "wikilink_space",     &o->wikilink_space);
  set_cstr(h, "wikilink_extension", &o->wikilink_extension);
  set_bool(h, "wikilink_sanitize",  &o->wikilink_sanitize);

  /* Stylesheet */
  set_bool(h, "embed_stylesheet", &o->embed_stylesheet);

  /* ARIA accessibility options */
  set_bool(h, "enable_aria", &o->enable_aria);

  /* Emoji options */
  set_bool(h, "enable_emoji_autocorrect", &o->enable_emoji_autocorrect);

  /* Terminal output */
  set_bool(h, "terminal_inline_images", &o->terminal_inline_images);
  set_int (h, "terminal_image_width",   &o->terminal_image_width);
  set_bool(h, "paginate",               &o->paginate);
  set_bool(h, "paginate_symbols",       &o->paginate_symbols);
  set_cstr(h, "theme_name",             &o->theme_name);
  set_int (h, "terminal_width",         &o->terminal_width);

  /* Syntax highlighting options */
  set_cstr(h, "code_highlighter",    &o->code_highlighter);
  set_bool(h, "code_line_numbers",   &o->code_line_numbers);
  set_bool(h, "highlight_language_only", &o->highlight_language_only);
  set_cstr(h, "code_highlight_theme", &o->code_highlight_theme);

  /* Marked / integration-specific options */
  set_bool(h, "enable_widont",           &o->enable_widont);
  set_bool(h, "code_is_poetry",          &o->code_is_poetry);
  set_bool(h, "enable_markdown_in_html", &o->enable_markdown_in_html);
  set_bool(h, "random_footnote_ids",     &o->random_footnote_ids);
  set_bool(h, "enable_hashtags",         &o->enable_hashtags);
  set_bool(h, "style_hashtags",          &o->style_hashtags);
  set_bool(h, "proofreader_mode",        &o->proofreader_mode);
  set_bool(h, "hr_page_break",           &o->hr_page_break);
  set_bool(h, "title_from_h1",           &o->title_from_h1);
  set_bool(h, "page_break_before_footnotes", &o->page_break_before_footnotes);

  set_cstr(h, "input_file_path", &o->input_file_path);
}

/*
 * Build options:
 * 1) mode defaults via apex_options_for_mode
 * 2) document front matter applied to options (library parity with CLI)
 * 3) explicit kwargs override front matter
 */
static apex_options
options_from_hash(VALUE h, const char *text, char ***bib_out, char ***conc_out,
                  apex_metadata_item **meta_out)
{
  *bib_out = NULL;
  *conc_out = NULL;
  *meta_out = NULL;

  apex_mode_t mode = APEX_MODE_UNIFIED;
  if (!NIL_P(h) && TYPE(h) == T_HASH) {
    VALUE mode_val = rb_hash_aref(h, ID2SYM(rb_intern("mode")));
    mode = mode_from_ruby(mode_val);
  }

  apex_options o = apex_options_for_mode(mode);

  /* Apply document metadata before kwargs so explicit options win. */
  if (text && (mode == APEX_MODE_MULTIMARKDOWN ||
               mode == APEX_MODE_KRAMDOWN ||
               apex_mode_is_unified_family(mode))) {
    char *work = strdup(text);
    if (work) {
      char *ptr = work;
      apex_metadata_item *meta = apex_extract_metadata_for_mode(&ptr, mode);
      if (meta) {
        apex_apply_metadata_to_options(meta, &o);
        *meta_out = meta;
      }
      free(work);
    }
  }

  if (!NIL_P(h) && TYPE(h) == T_HASH) {
    apply_kwargs(h, &o, bib_out, conc_out);
  }

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

  /* NUL-terminate for metadata extract helpers that expect C strings */
  char *text_copy = malloc(len + 1);
  if (!text_copy) return Qnil;
  memcpy(text_copy, c_text, len);
  text_copy[len] = '\0';

  char **bib = NULL;
  char **conc = NULL;
  apex_metadata_item *meta = NULL;
  apex_options opts = options_from_hash(opts_hash, text_copy, &bib, &conc, &meta);

  char *out = apex_markdown_to_html(c_text, len, &opts);
  free(text_copy);
  free_cstr_array(bib);
  free_cstr_array(conc);
  if (meta) apex_free_metadata(meta);

  if (!out) return Qnil;

  VALUE rb_out = rb_utf8_str_new_cstr(out);
  apex_free_string(out);
  return rb_out;
}

/*
 * call-seq:
 *   Apex::Native.version -> String
 *
 * Underlying Apex C library version string.
 */
static VALUE
rb_apex_version(VALUE self)
{
  return rb_utf8_str_new_cstr(APEX_VERSION_STRING);
}

void
Init_apex_ext(void)
{
  mApex       = rb_define_module("Apex");
  mApexNative = rb_define_module_under(mApex, "Native");

  rb_define_singleton_method(mApexNative, "markdown_to_html",
                             rb_apex_markdown_to_html, -1);
  rb_define_singleton_method(mApexNative, "version",
                             rb_apex_version, 0);
}
