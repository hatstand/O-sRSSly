function WebclipGetXpath(element) {
	var xpath = "";

	// Generate xpath
	do {
		if (element.nodeName == "HTML")
			break;

		var sibling = element;
		var siblings = 0;
		while (sibling = sibling.previousSibling)
			siblings++;

		xpath = element.nodeName + "[" + siblings + "]" + "," + xpath;
	} while (element = element.parentNode); 

	return xpath;
}

function WebclipGetRect(element) {
	var l = WebclipGetAbsoluteLeft(element);
	var t = WebclipGetAbsoluteTop(element) + element.offsetHeight;
	var rect = "";

	rect += l + ",";
	rect += t + ",";
	rect += element.offsetWidth + ",";
	rect += element.offsetHeight;

	return rect;
}

function WebclipGetElement(xpath) {
	// parse xpath
	var nodes = xpath.split(",");
	var element = document.getElementsByTagName("HTML")[0];
	for (var i = 0; i < nodes.length; ++i) {
		var node = nodes[i];
		if (node.length > 0) {
			var split_up = node.split("[");
			var nodename = split_up[0];
			var num = split_up[1].slice(0, -1);

			element = element.childNodes[num];
		}
	}

	return WebclipGetRect(element);
}

