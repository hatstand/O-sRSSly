/*
 * Copyright 2008 Google Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

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

