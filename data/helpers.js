// Stolen from GWT

function WebclipGetAbsoluteLeft(elem) {
  // Unattached elements and elements (or their ancestors) with style
  // 'display: none' have no offsetLeft.
  if (elem.offsetLeft == null) {
    return 0;
  }

  var left = 0;
  var curr = elem.parentNode;
  if (curr) {
    // This intentionally excludes body which has a null offsetParent.
    while (curr.offsetParent) {
      left -= curr.scrollLeft;
      curr = curr.parentNode;
    }
  }
    
  while (elem) {
    left += elem.offsetLeft;

    // Safari 3 does not include borders with offsetLeft, so we need to add
    // the borders of the parent manually.
    var parent = elem.offsetParent;
    if (parent && window.devicePixelRatio) {
      left += parseInt(document.defaultView.getComputedStyle(parent, '').getPropertyValue('border-left-width'));
    }

    // Safari bug: a top-level absolutely positioned element includes the
    // body's offset position already.
    if (parent && (parent.tagName == 'BODY') &&
        (elem.style.position == 'absolute')) {
      break;
    }

    elem = parent;
  }
  return left;
}

function WebclipGetAbsoluteTop(elem) {
  // Unattached elements and elements (or their ancestors) with style
  // 'display: none' have no offsetTop.
  if (elem.offsetTop == null) {
    return 0;
  }

  var top = 0;
  var curr = elem.parentNode;
  if (curr) {
    // This intentionally excludes body which has a null offsetParent.
    while (curr.offsetParent) {
      top -= curr.scrollTop;
      curr = curr.parentNode;
    }
  }
    
  while (elem) {
    top += elem.offsetTop;

    // Safari 3 does not include borders with offsetTop, so we need to add the
    // borders of the parent manually.
    var parent = elem.offsetParent;
    if (parent && window.devicePixelRatio) {
      top += parseInt(document.defaultView.getComputedStyle(parent, '').getPropertyValue('border-top-width'));
    }

    // Safari bug: a top-level absolutely positioned element includes the
    // body's offset position already.
    if (parent && (parent.tagName == 'BODY') &&
      (elem.style.position == 'absolute')) {
      break;
    }

    elem = parent;
  }
  return top;
}

