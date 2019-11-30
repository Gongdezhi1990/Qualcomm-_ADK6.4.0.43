############################################################################
# CONFIDENTIAL
#
# Copyright (c) 2014 - 2018 Qualcomm Technologies, Inc. and/or its
# subsidiaries. All rights reserved.
#
############################################################################
"""
Module used to output the information to a html file.
"""
import codecs
import re
import sys
import traceback

from ACAT.Display.Formatter import Formatter

# html tags for the nicer look

# All of this menu display originates from BlueCore CoreTools at
# //depot/bc/core_tools/CoreTools/Document/Document.pm

HTML_DOCUMENT_BEGINNING = """<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<style type="text/css">
.manualfmt {
  white-space: pre;
}
.alert:before {
  content: "@ ALERT: ";
  color: orange;
}
.error:before {
  content: "@@@@ ERROR: ";
  color: red;
}
.report, .alert, .error {
  margin-top: 0.5em;
  margin-bottom: 0.5em;
}
.menu_container {
  position: fixed;
  top: 0.5em;
  right: 0.5em;
  max-height: 95%;
  overflow: auto;
  overflow-x: hidden;
  z-index: 10;
  display: none
}
@media screen {
  .menu_container {
    display: block;
  }
}
.menu {
  background: #ffa;
  border: thin solid #f84;
  padding: 0.25em;
  position: relative;
  left: -0.25em;
  top: -0.25em;
  font-size: small;
  min-width: 6em;
}
.menu_shadow {
  position: relative;
  left: 0.25em;
  top: 0.25em;
  background: #888;
  margin-right: 0.25em;
  margin-bottom: 0.25em;
}
.menu_button_right, .open:before, .closed:before {
  width: 1em;
  height: 1em;
  background: #fc8;
  border: thin outset #f84;
  margin: 0.1em;
  padding: 0.1em;
  text-align: center;
  text-decoration: none;
  font-family: sans-serif;
  font-size: x-small;
  font-weight: bold;
  color: black;
}
.menu_button_right, .open, .closed {
  cursor: pointer;
}
.menu_button_right {
  display: block;
}
.menu_button_right {
  float: right;
  margin-left: 0.25em;
}
.menu_button_right:active, .open:active:before, .closed:active:before {
  border: thin inset #f84;
  color: black;
}
.menu ul {
  list-style: none;
  margin-left: 0.5em;
  padding: 0em;
}
.menu > ul {
  margin: 0em;
  clear: both;
  min-width: 30em;
}
.open:before {
  content: "^";
}
.closed:before {
  content: "v";
}
.open + ul {
  display: block;
}
.closed + ul {
  display: none;
}
</style>
<!--
IE 6 doesn't handle position: fixed properly. There are workarounds but
they're unpleasant. IE 6 is long in the tooth and menus aren't essential so
let's just forget about them
-->
<!--[if lt IE 7]>
<style type="text/css">
@media screen {
  .menu_container {
    display: none;
  }
}
</style>
<![endif]-->
<!--
Overrides for IE7 and later.

IE, correctly according to the CSS standard, puts scroll bars inside the
object. Other browsers incorrectly put them outside. Outside is what I want
here. So, for IE we'll add some padding for the scroll bars to go into. 20
pixels does it for me but clearly that's a bit fragile.

IE, however, doesn't resize the index just to contain the text it needs.
So, we have to specify a maximum width which IE then uses as the width.
This means the index isn't quite as small as it could be when it's all
folded up.
-->
<!--[if IE]>
<style type="text/css">
html .menu_container {
  padding-bottom: 5px; /* This helps too. */
  padding-right: 20px;
  overflow-x: hidden;
  overflow-y: auto;
  max-width: 33%;
}
</style>
<![endif]-->
<script language="javascript" type="text/javascript">
function menu_flip() {
 var el = document.getElementById("menu") ;
 if (el.style.right == 'auto') {
   el.style.right = '0.5em';
   el.style.left = 'auto';
 } else {
   el.style.right = 'auto';
   el.style.left = '0.5em';
 }
}
function menu_button(el)
{
  el.className = (el.className == "open") ? "closed" : "open";
}
function add_menu_folds(el)
{
  if (!el)
    return;

  for(var c = el.firstElementChild; c; c = c.nextElementSibling) {
    add_menu_folds(c);
    if (c.tagName == "UL") {
      var button = document.createElement("A");
      button.className = "closed";
      button.onclick = function() { menu_button(button) };
      el.insertBefore(button, c);
    }
  }
}
function loaded() {
  add_menu_folds(document.getElementById("menu"));
}
</script>
</head>
<body onload="loaded()">
"""

HTML_DOCUMENT_END = """</body>
</html>
"""


def html_simple_element(string, tag, attr_str=""):
    """
    Creates a simple HTML element containing the input string.
    """
    if attr_str != "":
        attr_str = " " + attr_str

    try:
        # If the given string is in bytes, convert it to string
        string = string.decode()

    except AttributeError:
        # When the `string' variable is already in string, it's expected
        # to see this exception.
        pass

    return "<" + tag + attr_str + ">" + string + "</" + tag + ">"


def html_heading(string, level):
    """
    Creates a html heading.
    """
    return html_simple_element(string, "h" + str(level)) + "\n"


def html_line(size):
    """
    Creates a html line
    """
    return '<hr noshade size="' + str(size) + '">'


def html_line_before(string, size):
    """
    Creates a html line before the string.
    """
    return html_line(size) + string


def html_div(string, cls):
    """
    Helper function to create special html elements like report, error,
    alert etc.
    """
    return html_simple_element(string, "div", 'class="%s"' % cls) + "\n"


def html_report(string):
    """
    Creates a html report element.
    """
    return html_div(string, "report")


def html_alert(string):
    """
    Creates a html alert.
    """
    return html_div(string, "alert")


def html_error(string):
    """
    Creates a html error.
    """
    return html_div(string, "error")


def html_manual_format(string):
    """
    Someone has manually formatted a section but we don't think they want
    monospaced text
    """
    return html_div(string, "manualfmt")


def html_pre(string):
    """
    Creates an unformated text section.
    """
    return html_simple_element(string, "pre")


def normalise_tag_id(input_id):
    """
    Removes special characters from the tag ID.
    """
    return input_id.replace(" ", "").replace("\n", "").replace("\r", "")


def html_tag(string, input_id, proc):
    """
    Creates a html tag which can be used to jump to a specific section.
    """
    return html_simple_element(
        string, "a", 'id="' + proc + "_" + normalise_tag_id(input_id) + '"'
    )


def html_link_to_tag(string, input_id, proc):
    """
    Creates a link to a specific tag
    """
    return html_simple_element(
        string, "a", 'href="#' + proc + "_" + normalise_tag_id(input_id) + '"'
    )


def html_list_item(string):
    """
    Creates a html list.
    """
    if string == "":
        return ""

    return "<li>" + string + "\n"


def html_unordered_list(items):
    """
    Creates an unordered html list.
    """
    if not items:
        return ""

    inner = "".join(map(html_list_item, items))
    if inner == "":
        return ""

    return "<ul>\n" + inner + "</ul>\n"


def plain_to_html(plain_text):
    """
    Helper function to display plain text. Replaces all the special characters.
    """
    if plain_text is None:
        return ""

    # We remove trailing whitespace - most notably newlines - so we
    # don't end up with unexpected vertical whitespace in the output.
    html = str(plain_text).rstrip().replace("&", "&amp;"). \
        replace("<", "&lt;").replace(">", "&gt;")

    # *Sigh* see if we can work out what they wanted.
    #
    # If they use more than one space anywhere except at the start or
    # end of a line, then they probably wanted fully preformatted text
    # with a monospaced font. We'll also look for the line drawing
    # character "|" and a common way tables are done (" : " after some
    # text on the line).
    #
    # Note that if output_list() was used then the lines in the list will be
    # joined together with newlines before getting here so they will be
    # treated as preformatted.
    #
    # Otherwise if they've used spaces at the start of a line or they've
    # used newlines then assume they want to control indentation and line
    # breaks but they don't need a monospaced font.
    #
    # If neither of those are true, then we have a simple piece of HTML.
    #
    # Since all output goes through plain_to_html on entry, that provides a
    # single entry point to change things later. For example, it could be
    # adapted so that if it is given a dictionary then an html key gives the
    # HTML rendering of the object or a pre key indicates that it's
    # preformatted text. Similarly, passing in a object could defer the
    # rendering to that object. This is the solution BlueCore's CoreTools
    # uses. Other solutions are possible.
    if re.search(r"\S {2,}\S", html) is not None or "|" in html or \
            re.search(r"\S.* : ", html) is not None:
        return html_pre(html)

    if re.search(r"^ ", html, flags=re.MULTILINE) or "\n" in html:
        return html_manual_format(html)

    return html


class Section(object):
    """
    @brief Class to encapsulate a section. In html a section is represented as
    header followed by a series of divs.
    """

    def __init__(self, title, parent=None, level=1, proc='P0'):
        """
        @brief Initialises the object.
        @param[in] self Pointer to the current object
        @param[in] title Plain text title
        @param[in] parent Pointer to the parent object
        @param[in] level Nesting depth of this object
        @param[in] current processor being documented

        Create a new section. When called publicly, the parent and level
        parameters must not be given and the result will be a top level
        section. The parameters are filled in when creating subsections which
        users of this class should do by calling the subsection method on an
        existing section.
        """
        # A section can have subsections this relation is represented by the
        # reference to the link
        self.parent = parent
        # how deepness of the section
        self.level = level
        self.title = title
        self.content = []  # No subsections in here
        self.subsections = []
        self.proc = proc

    def subsection(self, title):
        """
        @brief Create a subsection of this section
        @param[in] self Pointer to the current object
        @param[in] title Plain text title

        This creates a new section as a subsection of this one. Once that has
        been done, the add_report method of this section must not be called.
        It's still safe to call add_alert and add_error.
        """
        section = self.__class__(title, self, self.level + 1, proc=self.proc)
        self.subsections.append(section)
        return section

    # Private method that has no checking. Public methods that call this do
    # the approriate checking depending on the situation.
    def _add_content(self, content):
        self.content.append(content)

    # Alerts and errors can be detected quite late: after we've created
    # subsections. We don't want to emit a warning in these cases.
    def add_alert(self, content):
        """
        @brief Function used for adding an alert to the report
        @param[in] self Pointer to the current object
        @param[in] content HTML of the alert

        Note that this function handles just the in-line alert message. The
        links at the top of the document are handled by the caller.
        """
        self._add_content(html_alert(content))

    def add_error(self, content):
        """
        @brief Function used for adding an error report to the report
        @param[in] self Pointer to the current object
        @param[in] content HTML of the error message

        Note that this function handles just the in-line error message. The
        links at the top of the document are handled by the caller.
        """
        self._add_content(html_error(content))

    def add_report(self, content):
        """
        @brief Function used for adding content to the report
        @param[in] self Pointer to the current object
        @param[in] content HTML to be added to the report
        """
        if self.subsections:
            sys.stderr.write(
                (
                    'Text "%s" was added to section %s after subsection %s was '
                    'created, text will appear in the section but before '
                    'any subsections.\n'
                ) % (
                    content, '"' + self.title + '"'
                    if self.title != "" else "<unnamed>",
                    '"' + self.subsections[-1].title + '"'
                    if self.subsections[-1].title != "" else "<unnamed>"
                )
            )
            traceback.print_stack()

        self._add_content(html_report(content))

    def get_links(self):
        """
        @brief Generate the menu pertaining to this section
        @param[in] self Pointer to the current object
        """
        links = ""
        if self.title != "":
            links += html_link_to_tag(
                plain_to_html(self.title), self.title, self.proc
            )
        return links + \
            html_unordered_list([x.get_links() for x in self.subsections])

    def content_to_html(self):
        """
        @brief Convert section to html format.
        @param[in] self Pointer to the current object

        Note that this always returns either an empty string or a string
        containing one or more HTML block level element so it must not be
        wrapped in anything that can't contain such elements (most notably <p>
        tags).
        """
        if self.title != "":
            string_title = html_tag(
                plain_to_html(self.title), self.title, self.proc
            )
            string_title = html_heading(string_title, self.level)
        else:
            string_title = html_heading(html_line("1"), self.level)

        if self.level == 1:  # it's not a  sub-analysis
            string_title = html_line_before(string_title, "5")

        # We render all our content before all our subsections to stop any of
        # our content looking like it belongs to the subsection.
        string_content = "".join(self.content)
        for section in self.subsections:
            string_content += section.content_to_html()

        return string_title + string_content

    def get_parent(self):
        """
        @brief Returns the parent of this section
        @param[in] self Pointer to the current object
        """
        return self.parent


class HtmlFormatter(Formatter):
    """
    @brief Implements the Formatter interface to provide html output. Creates
    table of content for each section. Each section is displayed in different
    html section with a title. The sections are separated with lines.
    """

    def __init__(self, file_path):
        """
        @brief Initialises the object.
        @param[in] self Pointer to the current object
        @param[in] file_path
        """

        # All the lines we want to output
        self.sections = []
        # active section
        self.active_section = None

        # Alerts and errors get saved up so that we can highlight them at
        # the end.
        self.alerts = []
        self.errors = []

        self.file_path = file_path
        super(HtmlFormatter, self).__init__()

    def section_start(self, header_str):
        """
        @brief Starts a new section. Sections can be nested.
        @param[in] self Pointer to the current object
        @param[in] header_str
        """
        # we should remove any newline character from the header
        header_str = header_str.replace('\n|\r', '')
        if self.active_section is None:
            self.active_section = Section(header_str, proc=self.proc)
            self.sections.append(self.active_section)
        else:
            self.active_section = self.active_section.subsection(header_str)

    def section_end(self):
        """
        @brief End a section
        @param[in] self Pointer to the current object
        """
        if self.active_section:
            self.active_section = self.active_section.get_parent()
        else:
            print(
                "Section end called without active section. This is caused " +
                "by multiple section_end() calls."
            )

    def section_reset(self):
        """
        @brief Reset the section formatting, ending all open sections.
        This is provided so that in case of an error we can continue safely.
        @param[in] self Pointer to the current object
        """
        self.active_section = None

    @property
    def _output_object(self):
        if self.active_section is None:
            # This shouldn't happen. But rather than lose the text, or writing
            # it into some random section, let's capture it cleanly. If we ever
            # do a reset as part of capturing an exception then we may end up
            # here.
            self.section_start("Lost output")

        return self.active_section

    def output(self, string_to_output):
        """
        @brief Normal body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """

        html = plain_to_html(string_to_output)
        if html == "":
            return

        self._output_object.add_report(html)

    def output_svg(self, string_to_output):
        """
        @brief Normal body text. Lists/dictionaries will be compacted.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        self._output_object.add_report(string_to_output)

    def output_raw(self, string_to_output):
        """
        @brief Unformatted text output. Useful when displaying tables.
        @param[in] self Pointer to the current object
        @param[in] string_to_output
        """
        html = plain_to_html(string_to_output)
        if html == "":
            return
        html_pre_output = html_pre(html)

        self._output_object.add_report(html_pre_output)


    def alert(self, alert_str):
        """
        @brief Raise a alert - important information that we want to be
            highlighted.
        For example, 'pmalloc pools exhausted' or 'chip has panicked'.
        @param[in] self Pointer to the current object
        @param[in] alert_str
        """
        # Make sure alerts have the same type
        alert_str = str(alert_str)
        self._output_object.add_alert(
            html_tag(plain_to_html(alert_str), alert_str, self.proc)
        )
        self.alerts.append((alert_str, self.proc))

    def error(self, error_str):
        """
        @brief Raise an error. This signifies some problem with the analysis tool
        itself, e.g. an analysis can't complete for some reason.
        @param[in] self Pointer to the current object
        @param[in] error_str
        """
        # Make sure errors have the same type
        error_str = str(error_str)
        self._output_object.add_error(
            html_tag(plain_to_html(error_str), error_str, self.proc)
        )
        self.errors.append((error_str, self.proc))

    def flush(self):
        """
        @brief Output all logged events (body text, alerts, errors etc.) to
            the given file.
        @param[in] self Pointer to the current object
        """

        # menu
        html_text = """<div class="menu_container" id="menu">
  <div class="menu_shadow">
    <div class="menu">
      <a class="menu_button_right" href="javascript:menu_flip()">&harr;</a>
      Index
"""
        links = []
        for section in self.sections:
            links.append(section.get_links())
        html_text += \
            html_unordered_list(links)

        html_text += "\n    </div>\n  </div>\n</div>\n"

        if self.errors:
            # links to errors
            html_text += html_heading("Errors", 1)
            html_text += html_unordered_list(
                [
                    html_link_to_tag(plain_to_html(x[0]), x[0], x[1])
                    for x in sorted(self.errors)
                ]
            )

        if self.alerts:
            # links to alerts
            html_text += html_heading("Alerts", 1)
            html_text += html_unordered_list(
                [
                    html_link_to_tag(plain_to_html(x[0]), x[0], x[1])
                    for x in sorted(self.alerts)
                ]
            )

        # result of the analysis
        for sec in self.sections:
            html_text += sec.content_to_html()

        # write to file
        with codecs.open(self.file_path, 'w', encoding='utf8') as html_file:
            html_file.write(HTML_DOCUMENT_BEGINNING)
            html_file.write(html_text)
            html_file.write(HTML_DOCUMENT_END)
