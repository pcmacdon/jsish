// Author: Filip Stanek
// Website: http://www.bloodforge.com
// If you find a bug in the code below, please check to make sure you have the latest version ( http://www.bloodforge.com/myuploads/autocomplete.js )
// If the latest version has a bug, please post a comment on my website ( http://www.bloodforge.com/post/Flexible-AutoComplete-(Suggested-Words)-Code-for-Text-Input-Fields-using-JavaScript.aspx ) so that I can fix the problem.

/************ AutoCompleteGroup ************/
/// This method stores the suggested words for a group (defined by the autoComplete
/// attribute of the <input>).  It also stores the class which should be used for rendering
/// the DropDown.
///
/// I won't bother with giving a description of the members of AutoCompleteGroup.
/// If you want to modify this code, this should be very self-explanatory.
function AutoCompleteGroup($grp, $items, $style) {
    this._group = $grp;
    this._items = $items;
    if ($style != undefined && $style != "") {
        this._style = $style;
    }
    else {
        this._style = "defaultAutoCompletePopupStyle";
    }
}
AutoCompleteGroup.prototype._group;
AutoCompleteGroup.prototype._items;
AutoCompleteGroup.prototype._style;
AutoCompleteGroup.prototype.getGroup = function() {
    return this._group;
}
AutoCompleteGroup.prototype.getItems = function() {
    return this._items;
}
AutoCompleteGroup.prototype.setItems = function($items) {
    this._items = $items;
}
AutoCompleteGroup.prototype.getStyle = function() {
    return this._style;
}
AutoCompleteGroup.prototype.setStyle = function($style) {
    this._style = $style;
}

/************ AutoComplete ************/
/// Attach the Init method of the class to the window.onload event.
var autoComplete = new AutoComplete();
if (window.addEventListener) {
    window.addEventListener("load", function(e) { autoComplete.Init(); }, false);
}
else if (window.attachEvent) {
    window.attachEvent("onload", function(e) { autoComplete.Init(); });
}
else if (window.onload != null) {
    var onLd = window.onload;
    window.onload = function(e) {
        onLd(e);
        autoComplete.Init();
    }
}
else window.onload = function(e) { autoComplete.Init(); };


/// <summary>
/// The "class" responsible for display and control of the suggested words DropDown.
/// <summary>
function AutoComplete() {
    this._groups = new Array();
    this._cancelHide = false;
    this._autoSize = true;
    this._minTextLength = 0;
}

AutoComplete.prototype._groups; // stores the group and associated words that populate the dropdown list
AutoComplete.prototype._dropDown; // Object which contains the suggestions
AutoComplete.prototype._cancelHide;  // Used for canceling hiding
AutoComplete.prototype._autoSize; // if true, the DropDown will auto-size its width to match the associated text field
AutoComplete.prototype._minTextLength;

/// <summary>
/// Gets a list of <spans> that contain suggested words in the DropDown.
/// </summary>
/// <returns>A list of <span> objects that contain the suggested words.</returns>
AutoComplete.prototype.DropDownItems = function() {
    return this._dropdown.firstChild.childNodes;
}

/// <summary>
/// Checks whether the user is using IE
/// </summary>
/// <returns>True if the user is using IE, otherwise False</returns>
AutoComplete.prototype.isIE = function() {
    return navigator.appName == "Microsoft Internet Explorer";
}

/// <summary>
/// Sets autosizing of the drop down.
/// </summary>
/// <parameters>
///   <parameter name="$bSize" type="boolean">
///     If true, the drop down will match the width of the associated text field.  Otherwise, the drop down will use the width from the CSS style definition.
///     If this is not set, the default behavior is 'True'.
///   </parameter>
/// </parameters>
AutoComplete.prototype.AutoSize = function($bSize) {
    this._autoSize = $bSize;
}
AutoComplete.prototype.MinTextLength = function($bSize) {
    this._minTextLength = $bSize;
}
/// <summary>
/// Initializes the AutoComplete DropDown.
/// - Events are assigned to the text fields
/// - DropDown is created
/// </summary>
AutoComplete.prototype.Init = function() {
    this.CreateDefaultStyles();
    var inputs = document.getElementsByTagName("input");
    var myself = this;
    for (var iCount = 0; iCount < inputs.length; iCount++) {
        if (inputs[iCount].hasAttribute && inputs[iCount].hasAttribute("type") && inputs[iCount].getAttribute("type").toLowerCase() != "text") {
            continue;
        }
        var acGroup = inputs[iCount].getAttribute("autoComplete");
        if (acGroup != undefined) {
            var input = inputs[iCount];
            if (input.addEventListener) {
                input.addEventListener("keydown", function(e) { myself.OnKeyDown(this, e.which); }, false);
                input.addEventListener("blur", function(e) { myself.Hide(this); }, false);
                input.addEventListener("focus", function(e) {
                    if (this.value.length > 0) {
                        if (myself.ShowMatches(this.getAttribute("autoComplete"), this.value, this)) {
                            if (myself.DropDownItems()[0].firstChild.nodeValue != this.value) {
                                myself.Show(this);
                            }
                        }
                        else {
                            myself.Hide(this);
                        }
                    }
                }, false);
            }
            else if (input.attachEvent) {
                input.attachEvent("onkeydown", function(e) { myself.OnKeyDown(e.srcElement, window.event.keyCode); });
                input.attachEvent("onblur", function(e) { myself.Hide(e.srcElement); });
                input.attachEvent("onfocus", function(e) {
                    if (e.srcElement.value.length > 0) {
                        if (myself.ShowMatches(e.srcElement.getAttribute("autoComplete"), e.srcElement.value, e.srcElement)) {
                            if (myself.DropDownItems()[0].firstChild.nodeValue != e.srcElement.value) {
                                myself.Show(e.srcElement);
                            }
                        }
                        else {
                            myself.Hide(e.srcElement);
                        }
                    }
                });
            }
            else {
                input.onkeydown = function(e) {
                    var keynum = window.event ? window.event.keyCode : e.which;
                    myself.OnKeyDown(this, keynum);
                }
                input.onblur = function(e) {
                    myself.Hide(this);
                }
                input.onfocus = function(e) {
                    if (this.value.length > 0) {
                        if (myself.ShowMatches(this.getAttribute("autoComplete"), this.value, this)) {
                            if (myself.DropDownItems()[0].firstChild.nodeValue != this.value) {
                                myself.Show(this);
                            }
                        }
                        else {
                            myself.Hide(this);
                        }
                    }
                }
            }
        }
    }
    this._dropdown = document.createElement("div");
    this._dropdown.style.display = "none";

    var pnl = document.createElement("div");
    if (pnl.addEventListener) {
        document.addEventListener("mouseup", function(e) { myself.OnListRelease(e); }, false);
        pnl.addEventListener("mousedown", function(e) { myself.OnListClick(e); }, false);
    }
    else if (pnl.attachEvent) {
        document.attachEvent("onmouseup", function(e) { myself.OnListRelease(e); });
        pnl.attachEvent("onmousedown", function(e) { myself.OnListClick(e); });
    }
    else {
        pnl.onmouseup = function(e) {
            myself.OnListRelease(e);
        }
        pnl.onmousedown = function(e) {
            myself.OnListClick(e);
        }
    }
    this._dropdown.appendChild(pnl);

    document.body.appendChild(this._dropdown);
}

/// <summary>
/// Generates a default style sheet for the AutoComplete DropDown.
/// This stylesheet can be replaced by specifying a different CSS class in the Set() method (3rd parameter).
/// </summary>
AutoComplete.prototype.CreateDefaultStyles = function() {

    var style = document.createElement("style");
    style.type = "text/css";
    style.media = "screen";
    var styleStr = "";
    styleStr += ".defaultAutoCompletePopupStyle { font-family:Arial; font-size: 10pt; color:black; width: 150px; margin: 3px; background-color: Gray; }\n";
    styleStr += ".defaultAutoCompletePopupStyle div { top: -3px; left: -3px; background: white; border: solid 1px black; position: relative; overflow: auto; max-height: 100px; }\n";
    styleStr += ".defaultAutoCompletePopupStyle span { cursor: pointer; margin: 2px; display: block; }\n";
    styleStr += ".defaultAutoCompletePopupStyle span.wordSelected { background: blue;   color: White; }\n";

    if (this.isIE()) {
        styleStr += ".defaultAutoCompletePopupStyle { margin: 0; padding: 0 7px 7px 0; filter:shadow(color:gray, strength:6, direction:135); background-color: Transparent; }\n";
        styleStr += ".defaultAutoCompletePopupStyle div { top: 0; left: 0; height: expression(this.scrollHeight > 100? '100px' : 'auto'); }\n";
    }

    if (style.styleSheet) {
        style.styleSheet.cssText = styleStr;
    }
    else {
        style.appendChild(document.createTextNode(styleStr));
    }

    var head = (document.getElementsByTagName("head").length > 0) ? document.getElementsByTagName("head")[0] : document.body;
    head.appendChild(style);
}

/// <summary>
/// This method is called when the user clicks on the DropDown.
/// We need to cancel the hiding here, since in some browsers this will trigger the "blur" event of the text field.
/// </summary>
AutoComplete.prototype.OnListClick = function($e) {
    this._cancelHide = true;
}

/// <summary>
/// This method is called on the "mouseup" event of the document.  
/// It lets us know that we can now safely hide the DropDown when the user changes focus from the text field.
/// </summary>
AutoComplete.prototype.OnListRelease = function($e) {
    this._cancelHide = false;
}

/// <summary>
/// Use this method to set the words which should be suggested when the user types in the text field.
/// </summary>
/// <parameters>
///   <parameter name="$grp" type="string">
///      The group of the text field.  Each text field that uses this code must be associated with a group.
///      To associate a text field with a group, you must define the group as an attribute of the text field in the HTML.
///      For example:
///        <input type="text" id="txt1" autoComplete="grp1" />
///      In the example above, the name of the group would be 'grp1'.
///   </parameter>
///   <parameter name="$items" type="array">
///      The words which will appear as suggestions when the user types text into the text field of this group.
///   </parameter>
///   <parameter name="$style" type="string">
///      If specified, this will override the defaultAutoCompletePopupStyle for the DropDown for
///      all text fields associated with this group.
///   </parameter>
/// </parameters>
AutoComplete.prototype.Set = function($grp, $items, $style) {
    $items = $items.sort();
    for (var gCount = 0; gCount < this._groups.length; gCount++) {
        if (this._groups[gCount].getGroup() == $grp) {
            this._groups[gCount].setItems($items);
            if($style != null)
                this._groups[gCount].setStyle($style);
            return;
        }
    }
    var acGroup = new AutoCompleteGroup($grp, $items, $style);
    this._groups.push(acGroup);
}

/// <summary>
/// Helper method to populate the DropDown if any matching words were found.
/// </summary>
/// <parameters>
///   <parameter name="$grp" type="string">
///     The group of the text field specified by the autoComplete attribute.
///   </parameter>
///   <parameter name="$text" type="string">
///     The text in the text field.
///   </parameter>
///   <parameter name="$sender" type="HTMLInputElement">
///     The reference to the text field.
///   </parameter>
/// </parameters>
/// <returns>True if at least one word was found that matches the word specified in $text, otherwise False</returns>
AutoComplete.prototype.ShowMatches = function($grp, $text, $sender) {
    var matches = this.GetMatches($grp, $text);
    if (matches.length > 0) {
        this.PopulateDropDown(matches, $sender);
        return true;
    }
    return false;
}

/// <summary>
/// Gets all suggested words that correspond to the text in the text field.
/// </summary>
/// <parameters>
///   <parameter name="$grp" type="string">
///     The group of the text field specified by the autoComplete attribute.
///   </parameter>
///   <parameter name="$text" type="string">
///     The text in the text field.
///   </parameter>
/// </parameters>
/// <returns>An array of suggested words.</returns>
AutoComplete.prototype.GetMatches = function($grp, $text) {
    if ($text.length > this._minTextLength) {
        for (var gCount = 0; gCount < this._groups.length; gCount++) {
            if (this._groups[gCount].getGroup() == $grp) {
                var items = this._groups[gCount].getItems();
                var ncls = this._groups[gCount].getStyle();
                if (ncls) {
                    this._dropdown.className = ncls;
                }
                var matches = new Array();
                for (var iCount = 0; iCount < items.length; iCount++) {
                    if (items[iCount].toLowerCase().indexOf($text.toLowerCase()) == 0) {
                        matches.push(items[iCount]);
                    }
                }
                return matches;
            }
        }
    }
    return new Array();
}

/// <summary>
/// Adds the suggestions to the DropDown.
/// </summary>
/// <parameters>
///   <parameter name="$matches" type="array">
///      The array of suggested words that need to appear in the DropDown.
///   </parameter>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
/// </parameters>
AutoComplete.prototype.PopulateDropDown = function($matches, $sender) {
    while (this._dropdown.firstChild.hasChildNodes()) {
        this._dropdown.firstChild.removeChild(this.DropDownItems()[0]);
    }
    var myself = this;
    for (var mCount = 0; mCount < $matches.length; mCount++) {
        var sp = document.createElement("span");
        sp.appendChild(document.createTextNode($matches[mCount]));
        sp.onmouseover = function() {
            myself.OnMouseOver(this);
        }
        sp.onclick = function() {
            myself.OnMouseClick(this, $sender);
        }
        this._dropdown.firstChild.appendChild(sp);
    }
}

/// <summary>
/// Monitors key presses when the user types in an AutoComplete text field.
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
///   <parameter name="$keynum" type="number">
///     The character code of the key being typed.
///   </parameter>
/// </parameters>
AutoComplete.prototype.OnKeyDown = function($sender, $keynum) {
    var key = String.fromCharCode($keynum).toLowerCase();
    if ($keynum == 8)  // backspace
    {
        if ($sender.value.length > 0) {
            if (this.ShowMatches($sender.getAttribute("autoComplete"), $sender.value.substr(0, $sender.value.length - 1), $sender)) {
                this.Show($sender);
            }
            else {
                this.Hide($sender);
            }
        }
    }
    else if ($keynum == 27) // escape
    {
        this.Hide($sender);
    }
    else if ($keynum == 38) { // up arrow
        if (this.DropDownItems().length > 0 && this._dropdown.style.display == "block") {
            var position = 0;
            while (this.DropDownItems()[position] != undefined) {
                if (this.DropDownItems()[position].className == "wordSelected") {
                    this.DropDownItems()[position].className = "";
                    if (position > 0) {
                        this.DropDownItems()[position - 1].className = "wordSelected";
                        $sender.value = this.DropDownItems()[position - 1].firstChild.nodeValue;
                        this.FixScrollPosition(this.DropDownItems()[position - 1], false);
                    }
                    else {
                        this.DropDownItems()[this.DropDownItems().length - 1].className = "wordSelected";
                        $sender.value = this.DropDownItems()[this.DropDownItems().length - 1].firstChild.nodeValue;
                        this.FixScrollPosition(this.DropDownItems()[this.DropDownItems().length - 1], true);
                    }
                    return;
                }
                position++;
            }
            this.DropDownItems()[this.DropDownItems().length - 1].className = "wordSelected";
            $sender.value = this.DropDownItems()[this.DropDownItems().length - 1].firstChild.nodeValue;
            this.FixScrollPosition(this.DropDownItems()[this.DropDownItems().length - 1], true);
        }
    }
    else if ($keynum == 40) { // down arrow
        if (this.DropDownItems().length > 0 && this._dropdown.style.display == "block") {
            var position = 0;
            while (this.DropDownItems()[position] != undefined) {
                if (this.DropDownItems()[position].className == "wordSelected") {
                    this.DropDownItems()[position].className = "";
                    if (this.DropDownItems()[position + 1] != undefined) {
                        this.DropDownItems()[position + 1].className = "wordSelected";
                        $sender.value = this.DropDownItems()[position + 1].firstChild.nodeValue;
                        this.FixScrollPosition(this.DropDownItems()[position + 1], true);
                    }
                    else {
                        this.DropDownItems()[0].className = "wordSelected";
                        $sender.value = this.DropDownItems()[0].firstChild.nodeValue;
                        this._dropdown.firstChild.scrollTop = 0;
                    }
                    return;
                }
                position++;
            }
            this.DropDownItems()[0].className = "wordSelected";
            $sender.value = this.DropDownItems()[0].firstChild.nodeValue;
            this._dropdown.firstChild.scrollTop = 0;
        }
    }
    else if (("abcdefghijklmnopqrstuvwxyz0123456789`-=\\][;',./~!@#$%^&*()_+|}{\":?><").indexOf(key) > -1) {
        if (this.ShowMatches($sender.getAttribute("autoComplete"), $sender.value + key, $sender)) {
            this.Show($sender);
        }
        else {
            this.Hide($sender);
        }
    }
}

/// <summary>
/// Adjusts the scroll bar when the user navigates the DropDown with the up or down arrow keys.
/// </summary>
/// <parameters>
///   <parameter name="$item" type="HTMLElement">
///     The currently selected item in the DropDown.
///   </parameter>
///   <parameter name="$bShowAtBottom" type="boolean">
///     If true and the scroll bar needs to be adjusted, the item will appear as the bottom-most item of the DropDown,
///     otherwise, the item will appear as the top-most item in the DropDown after adjusting the scroll bar.
///   </parameter>
/// </parameters>
AutoComplete.prototype.FixScrollPosition = function($item, $bShowAtBottom) {
    var curScrollTop = this._dropdown.firstChild.scrollTop;
    var visHeight = this._dropdown.firstChild.offsetHeight ? this._dropdown.firstChild.offsetHeight : this._dropdown.firstChild.height;
    var itemY = $item.offsetTop ? $item.offsetTop : $item.y;
    var itemHeight = $item.offsetHeight ? $item.offsetHeight : $item.height;

    if ($bShowAtBottom) {
        if (curScrollTop + visHeight < itemY + itemHeight) {
            this._dropdown.firstChild.scrollTop = itemY + itemHeight - visHeight;
        }
    }
    else {
        if (curScrollTop > itemY) {
            this._dropdown.firstChild.scrollTop = itemY;
        }
    }
}

/// <summary>
/// Makes the DropDown visible.
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
/// </parameters>
AutoComplete.prototype.Show = function($sender) {
    this._dropdown.style.position = 'absolute';
    this._dropdown.style.top = (this.GetTop($sender) + 3) + "px";
    this._dropdown.style.left = (this.GetLeft($sender) + 2) + "px";
    if (this._autoSize) {
        if ($sender.offsetWidth) {
            this._dropdown.style.width = $sender.offsetWidth + "px";
        }
        else if ($sender.width) {
            this._dropdown.style.width = $sender.width + "px";
        }
    }
    var myself = this;
    var interval = setInterval(function() {
        myself._dropdown.style.display = "block";
        clearInterval(interval);
    }, 100);
}

/// <summary>
/// Makes the DropDown invisible.
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
/// </parameters>
AutoComplete.prototype.Hide = function($sender) {
    var myself = this;
    var hideInterval = setInterval(function() {
        if (!myself._cancelHide) {
            myself._dropdown.style.display = "none";
            clearInterval(hideInterval);
        }
    }, 100);
}

/// <summary>
/// Calculates the appropriate left offset for the DropDown
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
/// </parameters>
/// <returns>The left offset at which the dropdown should be displayed.</returns>
AutoComplete.prototype.GetLeft = function($sender) {
    var left = 0;
    if ($sender.offsetParent) {
        while ($sender.offsetParent) {
            left += $sender.offsetLeft;
            $sender = $sender.offsetParent;
        }
    }
    else if ($sender.x)
        left += $sender.x;
    return left;
}

/// <summary>
/// Calculates the appropriate top offset for the DropDown
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The text field being typed into.
///   </parameter>
/// </parameters>
/// <returns>The top offset at which the dropdown should be displayed.</returns>
AutoComplete.prototype.GetTop = function($sender) {
    var top = 0;
    if ($sender.offsetParent) {
        top += $sender.offsetHeight;
        while ($sender.offsetParent) {
            top += $sender.offsetTop;
            $sender = $sender.offsetParent;
        }
    }
    else if ($sender.y) {
        top += $sender.y;
        top += $sender.height;
    }
    return top;
}

/// <summary>
/// Gets called when the user moves the mouse over a suggested word in the drop down.
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The <span> elemented which contains the suggested word.
///   </parameter>
/// </parameters>
AutoComplete.prototype.OnMouseOver = function($sender) {
    var position = 0;
    while (this.DropDownItems()[position] != undefined) {
        this.DropDownItems()[position].className = "";
        position++;
    }
    $sender.className = "wordSelected";
}

/// <summary>
/// Gets called when the user clicks on a suggested word in the drop down.
/// </summary>
/// <parameters>
///   <parameter name="$sender" type="HTMLInputElement">
///     The <span> elemented which contains the suggested word.
///   </parameter>
/// </parameters>
AutoComplete.prototype.OnMouseClick = function($sender, $textfield) {
    this._cancelHide = false;
    $textfield.value = $sender.firstChild.nodeValue;
    $textfield.focus();
    this.Hide($textfield);
}

/// <summary>
/// Prints debug info to the window.  Much better than alert()!.
/// </summary>
function debug($str) {
    try {
        var sp = document.createElement("div");
        sp.appendChild(document.createTextNode($str));
        if (document.getElementById("debug")) {
            document.getElementById("debug").appendChild(sp);
        }
        else {
            var dbg = document.createElement("div");
            dbg.id = "debug";
            dbg.style.padding = "50px";
            dbg.style.border = "solid 1px black";
            dbg.style.background = "white";
            dbg.style.color = "black";
            dbg.appendChild(sp);
            document.body.appendChild(dbg);
        }
    }
    catch (e) { }
}
