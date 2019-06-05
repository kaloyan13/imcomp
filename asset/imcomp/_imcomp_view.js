/*
  Image Comparator (IMCOMP)
  www.robots.ox.ac.uk/~vgg/software/imcomp/

  Copyright (c) 2017-2018, Abhishek Dutta, Visual Geometry Group, Oxford University.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
*/

////////////////////////////////////////////////////////////////////////////////
//
// @file        _imcomp_view.js
// @description View of the MVC design for user interface javascript
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        July 2018
//
////////////////////////////////////////////////////////////////////////////////

function _imcomp_view() {
  this.now = {};
  //this.now[{'base','comp'}].zoom = {is_enabled, is_frozen...}

  this.cpanel = document.getElementById('top_panel'); // global control panel

  this.theme = {};
  this.theme.MSG_TIMEOUT_MS = 5000;
  this.theme.TOGGLE_SPEED = 300; // { 150, 300, 600 } ms
  this.theme.ZOOM_LEVEL = 2; // { 0.2, 0.5, 2, 3 } X
  this.theme.ZOOM_WINDOW_SIZE = 300; // in pixels

  this.message_panel = document.getElementById('message_panel');

  // capture all user interface events and notify _imcomp_controller
  this.handleEvent = function(e) {
    if( e.currentTarget.classList.contains('disabled') ) {
      return;
    }
    var action_id = e.currentTarget.id;
    if ( action_id.length >= 4 ) {
      var type = action_id.substr(0, 4);
    }

    switch(e.currentTarget.id) {
    case 'add_images':
      this.select_local_files();
      break;
    case 'base_move_to_next':
    case 'comp_move_to_next':
      this.c.move_to_next(type);
      break;
    case 'base_move_to_prev':
    case 'comp_move_to_prev':
      this.c.move_to_prev(type);
      break;

    case 'move_to_next_pair':
      this.c.move_to_next_pair();
      break;
    case 'move_to_prev_pair':
      this.c.move_to_prev_pair();
      break;

    case 'compare_base_comp':
      this.c.compare_base_comp();
      break;
    case 'save_result':
      this.c.save_result();

    default:
      console.log('_via_view: handler unknown for event: ' + e.currentTarget);
    }
    e.stopPropagation();
  }
}

_imcomp_view.prototype.init = function( imcomp_controller ) {
  this.c = imcomp_controller;

  for( var type in this.c.type_list ) {
    this.now[type] = {};
    this.now[type].zoom = {};
  }

  // set toggle speed to default value selection
  var toggle_speed_dropdown = document.getElementById('toggle_speed');
  for( var i=0; i < toggle_speed_dropdown.options.length; i++ ) {
    if( parseInt(toggle_speed_dropdown.options[i].value) == this.theme.TOGGLE_SPEED ) {
      toggle_speed_dropdown.selectedIndex = i;
    }
  }

  // set zoom level to default value selection
  var zoom_level_dropdown = document.getElementById('zoom_level');
  for( var i=0; i < zoom_level_dropdown.options.length; i++ ) {
    if( parseInt(zoom_level_dropdown.options[i].value) == this.theme.ZOOM_LEVEL ) {
      zoom_level_dropdown.selectedIndex = i;
    }
  }

  this.connect_ui_elements_to_imcomp_view();

  // precompute mostly used values
  this.theme.ZOOM_WINDOW_SIZE_BY2 = this.theme.ZOOM_WINDOW_SIZE / 2;
}

_imcomp_view.prototype.select_local_files = function() {
  this.local_file_selector = document.createElement('input');
  this.local_file_selector.setAttribute('id', 'local_file_selector');
  this.local_file_selector.setAttribute('type', 'file');
  this.local_file_selector.setAttribute('name', 'files[]');
  this.local_file_selector.setAttribute('multiple', 'multiple');
  this.local_file_selector.setAttribute('style', 'display:none;');
  //this.local_file_selector.classList.add('display-none');
  this.local_file_selector.setAttribute('accept', '.jpg,.jpeg,.png,.bmp');

  this.local_file_selector.addEventListener('change', function(e) {
    this.c.update_files(e);
  }.bind(this), false);
  this.local_file_selector.click();
}

_imcomp_view.prototype.connect_ui_elements_to_imcomp_view = function() {
  document.getElementById( 'add_images').addEventListener('click', this, false);

  for( var type in this.c.type_list ) {
    document.getElementById( type + '_move_to_prev').addEventListener('click', this, false);
    document.getElementById( type + '_move_to_next').addEventListener('click', this, false);
    document.getElementById( type + '_img_filename_list').addEventListener('change', function(e) {
      var type = e.target.id.substr(0,4);
      this.c.update_now(type, e);
    }.bind(this), false);
  }

  document.getElementById( 'move_to_prev_pair').addEventListener('click', this, false);
  document.getElementById( 'move_to_next_pair').addEventListener('click', this, false);

  document.getElementById( 'compare_base_comp').addEventListener('click', this, false);

  document.getElementById( 'toggle_speed').addEventListener('change', function(e) {
    this.theme.TOGGLE_SPEED = e.target.value;
  }.bind(this), false);

  document.getElementById( 'zoom_level').addEventListener('change', function(e) {
    this.theme.ZOOM_LEVEL = e.target.value;
    this.c.zoom_update_level();
  }.bind(this), false);

  // for files panel drag and drop
  document.getElementById('files_panel').addEventListener('dragstart', function(e) {
    e.dataTransfer.setData("text", e.target.id);
    console.log('id is ' + e.target.id);
  }.bind(this), false);

  document.getElementById( 'right_content_container' ).addEventListener('dragover', function(e) {
    e.preventDefault();
  }.bind(this), false);

  document.getElementById( 'left_content_container' ).addEventListener('dragover', function(e) {
    e.preventDefault();
  }.bind(this), false);

  document.getElementById('right_content_container').addEventListener('drop', function(e) {
    e.preventDefault();
    var fileid = e.dataTransfer.getData("text");
    console.log('fileid in right content container is ' + fileid);
    this.c.file_dropped(fileid, 'comp');
  }.bind(this), false);

  document.getElementById('left_content_container').addEventListener('drop', function(e) {
    e.preventDefault();
    var fileid = e.dataTransfer.getData("text");
    console.log('fileid in left content container is ' + fileid);
    this.c.file_dropped(fileid, 'base');
  }.bind(this), false);

  document.getElementById('save_result').addEventListener('click', this, false);

  // fade between right and left images
  document.getElementById('base_comp_fader').addEventListener('input', function(e) {
    this.c.update_base_comp_fader(e);
  }.bind(this), false);

  // slide action in result visualisation
  document.getElementById('slider').addEventListener('mousedown', function(e) {
    e.stopPropagation();
    this.c.slider_mousedown_handler(e);
  }.bind(this), false);
  // once the slider is pressed, we can move the mouse anywhere on the window to slide.
  // so binding the slider move event at the window level.
  window.addEventListener('mousemove', function(e) {
    this.c.slider_mousemove_handler(e);
  }.bind(this), false);
  window.addEventListener('mouseup', function(e) {
    this.c.slider_mouseup_handler(e);
  }.bind(this), false);

  // hover to switch between base and compared images
  document.getElementById('left_content_image').addEventListener('mouseover', function(e) {
    e.stopPropagation();
    this.c.hover_to_right(e);
  }.bind(this), false);
  document.getElementById('left_content_image').addEventListener('mouseleave', function(e) {
    e.stopPropagation();
    this.c.hover_to_left(e);
  }.bind(this), false);
}

_imcomp_view.prototype.msg = function(msg, t) {
  //console.log('showing msg: ' + msg);
  if ( this.message_clear_timer ) {
    clearTimeout(this.message_clear_timer); // stop any previous timeouts
  }
  this.message_panel.innerHTML = msg;

  if ( t !== 0 ) {
    var timeout = t || this.theme.MSG_TIMEOUT_MS;
    this.message_clear_timer = setTimeout( function() {
      this.message_panel.innerHTML = '&nbsp;';
    }.bind(this), timeout);
  }
}
