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
// @file        _traherne_controller.js
// @description Controller of the MVC design for user interface javascript
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        Nov 2017
//
////////////////////////////////////////////////////////////////////////////////

function _traherne_controller() {
  this.type_list = {'base':'left_content', 'comp':'right_content'};
  this.type_description = {'base':'Base', 'comp':'Comp.'};

  this.state = {};
/*
  this.now = {};
  this.now.base = {};
  this.now.comp = {};
*/

  this.config = {};
  this.config.upload = {};
  this.config.upload.MAX_IMG_DIM_PX = 1200;
  this.config.imcomp_server_upload_uri = "/imcomp/_upload";
  this.config.imcomp_server_compare_uri = "/imcomp/_compare";
  this.config.imcomp_server_transform_uri = "/imcomp/_transform";
  this.config.imcomp_server_file_uri = "/imcomp/_file";
  this.config.placeholder_tif_conv_ongoing = "/imcomp/img/tif_conversion_ongoing_message.png";
  this.config.placeholder_tif_conv_error = "/imcomp/img/tif_conversion_error_message.png";

  this.compare = {}
  this.compare.is_ongoing = false;
  this.compare.start_time = {};
  this.compare.end_time = {};
  this.compare.promise = {};  // promise to current compare operation
  this.compare.result = {};

  this.ref_line = {};
}

function _traherne_compare_instance(findex1, findex2) {
  this.findex1 = findex1;
  this.findex2 = findex2;
}

_traherne_controller.prototype.init = function( traherne_model, traherne_view ) {
  this.m = traherne_model;
  this.v = traherne_view;

  for(var type in this.type_list) {
    this.clear_content(type);
  }
  this.disable_all_content_selectors();
  this.disable_all_switches('_toggle');
  this.disable_all_switches('_zoom');

  this.init_ref_line();

  this.show_message('To start collation, click <span class="blue">Load Base</span> and <span class="blue">Load Comp</span> buttons to load base and compare images.');

  /*
  // for debugging zoom feature
  for( type in this.type_list ) {

    var sid_suffix = 'image';
    var via = document.getElementById( this.type_list[type] + '_via_panel' );
    var img = document.getElementById( this.type_list[type] + '_img_panel' );
    this.hide_element(via);
    this.show_element(img);
    this.clear_toggle( this.type_list[type] ); // clear any existing toggle
    this.disable_all_switches('_toggle');
    this.enable_switch(type, '_zoom'); // @todo: enable zoom for all images
    var content_url = '/imcomp/traherne/test_image1.jpg';
    var content_img = document.getElementById( this.type_list[type] + '_image' );
    content_img.setAttribute('src', content_url);
    this.content_selector_set_checked(type, sid_suffix);
    this.v.now[type].sid_suffix = sid_suffix;
  }
*/
}

_traherne_controller.prototype.reset_controller_state = function(type) {
  this.m.reset_model_state(type);
  this.clear_content(type);

  this.content_selector_group_set_state(type, false);
  this.disable_switch(type, '_toggle');
  this.disable_switch(type, '_zoom');
}

_traherne_controller.prototype.update_files = function(type, e) {
  this.reset_controller_state(type);
  this.m.add_images(type, e.target.files);
}

_traherne_controller.prototype.move_to_next = function(type) {
  var n = this.m.files[type].length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === (n - 1) ) {
      this.set_now(type, 0);
    } else {
      this.set_now(type, now_findex + 1);
    }
  }
}

_traherne_controller.prototype.move_to_prev = function(type) {
  var n = this.m.files[type].length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === 0 ) {
      this.set_now(type, n - 1);
    } else {
      this.set_now(type, now_findex - 1);
    }
  }
}

_traherne_controller.prototype.move_to_next_pair = function() {
  for( var type in this.type_list ) {
    this.move_to_next(type);
  }
}
_traherne_controller.prototype.move_to_prev_pair = function() {
  for( var type in this.type_list ) {
    this.move_to_prev(type);
  }
}

_traherne_controller.prototype.on_filelist_update = function(type) {
  this.update_view_filelist(type);
  this.set_now(type, 0); // show first image

  var sid_suffix = type + '_via';

  this.content_selector_set_state(type, sid_suffix, true);
  this.set_content(type, sid_suffix);

  this.enable_switch(type, '_zoom');

  if( this.m.file_count['base'] && this.m.file_count['comp'] ) {
    this.show_message('Now <span class="blue">select a region</span> in the base image. Keeping the right mouse button pressed on the base image, drag mouse cursor to select a region.');
  }
}

_traherne_controller.prototype.update_view_filelist = function(type) {
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
  list.innerText = null; // clear existing entries
  var filelist = this.m.get_filelist(type);
  var n = filelist.length;

  for( var i=0; i<n; i++ ) {
    var opt = document.createElement('option');
    opt.value = i;
    opt.text = '[' + (i+1) + '] ' + filelist[i];
    list.add(opt, null);
  }
}

_traherne_controller.prototype.set_now = function(type, findex) {
  this.m.via[type].c.load_file_from_index( findex );
  this.v.now[type].findex = findex;

  // update the content to show {base,comp} full image
  var sid_suffix = type + '_via';
  this.content_selector_set_state(type, sid_suffix, true);
  this.set_content(type, sid_suffix);

  this.on_now_update(type);
}

_traherne_controller.prototype.on_now_update = function(type) {
  // update the selected item in filelist dropdown
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
  var now_findex = this.v.now[type].findex;

  if( list.options.length ) {
    list.selectedIndex = now_findex;
    /*
    for ( var i=0; i<list.options.length; i++ ) {
      if ( parseInt(list.options[i].value, 10) === now_findex ) {
        list.options[i].setAttribute('selected', 'selected');
      } else {
        if ( list.options[i].hasAttribute('selected') ) {
          list.options[i].removeAttribute('selected')
        }
      }
    }
    */
  }
  this.update_now_file_status(type);
  this.clear_message();
}

_traherne_controller.prototype.update_now = function(type, e) {
  var findex = parseInt(e.target.value, 10);
  this.set_now(type, findex);
}

_traherne_controller.prototype.on_upload_status_update = function(type, findex) {
  if ( this.v.now[type].findex == findex ) {
    this.update_now_file_status(type);
  }
}

_traherne_controller.prototype.update_now_file_status = function(type) {
  var now_findex = this.v.now[type].findex;
  var status = this.m.upload_status[type][now_findex].status;
  var msg = this.m.upload_status[type][now_findex].msg;
  var status_symbol;
  switch(status) {
  case 'OK':
    status_symbol = '<span style="color:green">&#9745;</span>';
    break;
  case 'ERR':
    status_symbol = '<span style="color:red">&#9746;</span>';
    break;
  default:
    status_symbol = '&#9744;';
    break;
  }

  var elem = document.getElementById( type + '_img_status' );
  elem.innerHTML = '<span title="' + msg + '">' + status_symbol + '</span>';
}

_traherne_controller.prototype.is_upload_resize_req = function(type, findex, w, h) {
  if( w > this.config.upload.MAX_IMG_DIM_PX || h > this.config.upload.MAX_IMG_DIM_PX ) {
    return true;
  } else {
    return false;
  }
}

///
/// Comparison of Base and Comp. pair
///
_traherne_controller.prototype.compare_base_comp = function() {
  // sanity checks
  for( type in this.type_list ) {
    if( this.m.file_count[type] === 0 ) {
      var type_name = this.type_list[type];
      this.show_message('You must load base and comp images before starting the comparison process. Click <span class="blue">Load Base</span> and <span class="blue">Load Comp</span> button to load base and comp images.');
      return;
    }
  }

  if( !this.is_base_region_selected() ) {
    this.show_message('To compare, you must <span class="blue">select a region</span> in the base image. Keeping the right mouse button pressed on the base image, drag mouse cursor to select a region.');
    return;
  }

  if( this.compare.is_ongoing ) {
    this.show_message('Please wait, compare process is <span class="blue">ongoing</span> ...');
    return;
  }

  var findex1 = this.v.now['base'].findex;
  var findex2 = this.v.now['comp'].findex;
  var compare_task = new _traherne_compare_instance(findex1, findex2);

  this.m.upload['base'][findex1].then( function(result1) {
    if ( !result1.success ) {
      this.show_message('Cannot compare because file ' +
                        '[ ' + this.m.files[result1.type][result1.findex].name + ' ] ' +
                        '<span class="red">failed to upload</span>.');
      return;
    }
    compare_task.upload_id1 = result1.fid;
    compare_task.scale1 = this.m.upload_scale['base'][compare_task.findex1];
    var fid1 = this.m.index_to_fid['base'][compare_task.findex1];
    var rid = this.m.via['base'].v.now.all_rid_list[0];
    compare_task.region1 = this.m.via['base'].m.regions[fid1][rid].dimg.slice(0);

    this.m.upload['comp'][findex2].then( function(result2) {
      if ( !result2.success ) {
        this.show_message('Cannot compare because file ' +
                          '[ ' + this.m.files[result2.type][result2.findex].name + ' ] ' +
                          '<span class="red">failed to upload</span>.');
        return;
      }
      compare_task.upload_id2 = result2.fid;
      compare_task.scale2 = this.m.upload_scale['comp'][compare_task.findex2];
      this.compare.is_ongoing = true;
      this.compare.promise = this.m.compare_img_pair(compare_task);
      var exp_comp_time;
      var algorithm_choice = document.getElementById('algorithm_choice');
      var algname = algorithm_choice.options[algorithm_choice.selectedIndex].value;
      switch(algname) {
        case 'ransac_dlt':
          exp_comp_time = 5;
          break;
        case 'robust_ransac_tps':
          exp_comp_time = 10;
          break;
      }
      this.show_message('<span class="blue">Comparing ... </span>(Please wait, it takes around ' + exp_comp_time + ' sec. to complete, larger regions may take longer to complete)')
    }.bind(this), function(err2) {
      var filename = this.files[ err2.type ][ err2.findex].name;
      this.show_message('<span class="red">Cannot compare as image [' + filename + ' could not be uploaded to server');
    }.bind(this));
  }.bind(this), function(err1) {
      var filename = this.files[ err1.type ][ err1.findex].name;
    this.show_message('<span class="red">Cannot compare as image [' + filename + ' could not be uploaded to server');
  }.bind(this));
}

_traherne_controller.prototype.is_base_region_selected = function() {
  if ( this.m.via['base'].v.now.all_rid_list.length == 1 ) {
    return true;
  } else {
    return false;
  }
}

_traherne_controller.prototype.on_compare_start = function() {
  this.compare.start_time = new Date();
  document.getElementById('compare_base_comp').disabled = true;
}

_traherne_controller.prototype.on_compare_end = function() {
  this.compare.end_time = new Date();
  this.compare.is_ongoing = false;

  document.getElementById('compare_base_comp').disabled = false;

  this.compare.promise.then( function(c) {
    this.compare.result = c;
    if( c.response.status === 'OK' ) {
      this.on_compare_success();
    } else {
      this.on_compare_failure();
    }
  }.bind(this));
}

_traherne_controller.prototype.on_compare_status_update = function() {
  this.show_message('Compare status: ' + this.m.compare_status.msg);
}

_traherne_controller.prototype.on_compare_success = function() {
  // note: this.compare.result contains the result
  var time = this.compare.end_time - this.compare.start_time;
  this.show_message('Comparison <span class="blue">completed in ' + Math.round(time/1000) + ' sec.</span>');

  this.show_compare_result();

  this.enable_all_content_selectors();
}

_traherne_controller.prototype.on_compare_failure = function() {
  var time = this.compare.end_time - this.compare.start_time;
  this.show_message('<span class="red">Comparison failed!</span');
}

_traherne_controller.prototype.show_compare_result = function(c) {
  // NOTE: this.compare.result contains the comparison result
  // var c = this.compare.result.response;
  // c = {"homography", "file1_crop", "file2_crop", "file2_crop_tx", "file1_file2_diff"}
  var c = this.compare.result.response;
  // in base panel, show file1 crop
  this.set_content('base', 'base_crop');

  // in comp panel, show file2 transformed + crop
  this.set_content('comp', 'comp_crop_tx');
}

_traherne_controller.prototype.disable_all_content_selectors = function() {
  for( var type in this.type_list ) {
    this.content_selector_group_set_state(type, false);
  }
}

_traherne_controller.prototype.enable_all_content_selectors = function() {
  for( var type in this.type_list ) {
    this.content_selector_group_set_state(type, true);
  }
}

_traherne_controller.prototype.content_selector_set_state = function(type, sid_suffix, is_enabled) {
  var container_name_prefix = this.type_list[type];
  var content_selector_name = container_name_prefix + '_' + sid_suffix;
  var content_selector = document.getElementById(content_selector_name);
  if(is_enabled) {
    this.enable_content_selector(content_selector);
  } else {
    this.disable_content_selector(content_selector);
  }
}

_traherne_controller.prototype.content_selector_set_checked = function(type, name_suffix) {
  this.content_selector_uncheck_all(type);
  var container_name_prefix = this.type_list[type];
  var content_selector_name = container_name_prefix + '_' + name_suffix;
  var content_selector = document.getElementById(content_selector_name);
  content_selector.checked = true;
}

_traherne_controller.prototype.content_selector_uncheck_all = function(type) {
  var container_name = this.type_list[type] + '_selector';
  var container = document.getElementById(container_name);
  var child = container.getElementsByClassName('content_selector');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    child[i].checked = false;
  }
}

_traherne_controller.prototype.enable_content_selector = function(element) {
  element.removeAttribute('disabled');
  element.addEventListener('click', this.content_selector_event_handler.bind(this), false);
}

_traherne_controller.prototype.disable_content_selector = function(element) {
  element.setAttribute('disabled', 'disabled');
  element.removeEventListener('click', this.content_selector_event_handler.bind(this), false);
}

_traherne_controller.prototype.content_selector_group_set_state = function(type, is_enabled) {
  var container_name_prefix = this.type_list[type];
  var container_name = container_name_prefix + '_selector';
  var container = document.getElementById(container_name);
  var child = container.getElementsByClassName('content_selector');
  var n = child.length;

  for( var i=0; i<n; i++ ) {
    if(is_enabled) {
      this.enable_content_selector(child[i]);
    } else {
      this.disable_content_selector(child[i]);
    }
  }
}

_traherne_controller.prototype.content_selector_event_handler = function(e) {
  var id = e.target.id;
  var container_type = this.get_container_type(id);
  var container_name = this.type_list[container_type];
  var sid_suffix = id.substr(container_name.length + 1);
  this.set_content(container_type, sid_suffix);
}

_traherne_controller.prototype.clear_content = function(type) {
  var container_name = this.type_list[type] + '_container';
  var container = document.getElementById(container_name);
  var child = container.getElementsByClassName('content');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    this.hide_element(child[i]);
  }
  this.disable_image_zoom(type);
}

_traherne_controller.prototype.set_content = function(type, sid_suffix) {
  var via = document.getElementById( this.type_list[type] + '_via_panel' );
  var img = document.getElementById( this.type_list[type] + '_img_panel' );

  if( sid_suffix.endsWith('_via') ) {
    this.hide_element(img);
    this.show_element(via);
    this.disable_all_switches('_toggle'); // toggle requires image content

    // reload zoom contents if zoom switch is selected
    if( this.v.now[type].zoom.is_enabled ) {
      this.disable_image_zoom(type);
      this.v.now[type].zoom.is_enabled = true;
      this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
      this.m.via[type].c.zoom_activate();
    }
  } else {
    this.hide_element(via);
    this.show_element(img);
    this.clear_toggle( this.type_list[type] ); // clear any existing toggle
    this.enable_all_switches('_toggle');

    var content_url = this.get_content_url(type, sid_suffix);
    var img_content = document.getElementById( this.type_list[type] + '_image' );
    img_content.setAttribute('src', content_url);

    // reload zoom contents if zoom switch is selected
    if( this.v.now[type].zoom.is_enabled ) {
      this.m.via[type].c.zoom_deactivate();
      this.disable_image_zoom(type);
      this.enable_image_zoom(type);
    }

    if( sid_suffix === 'base_comp_diff') {
      //this.show_message('In the difference image, color code is as follows: <span style="color: #0072b2">base image</span> and <span style="color: #d55e00">comp. image</span>');
      this.show_message('In the difference image, color code is as follows: <span style="color: blue">base image</span> and <span style="color: red">comp. image</span>');
    }
  }
  this.enable_switch(type, '_zoom');
  this.content_selector_set_checked(type, sid_suffix);
  this.v.now[type].sid_suffix = sid_suffix;
}

_traherne_controller.prototype.get_content_url = function(type, sid_suffix) {
  // NOTE: this.compare.result contains the comparison result
  // var c = this.compare.result.response;
  // c = {"homography", "file1_crop", "file2_crop", "file2_crop_tx", "file1_file2_diff"}

  var content_url = '';
  switch(sid_suffix) {
  case 'base_via':
  case 'comp_via':
    content_url = 'via_panel';
    break;
  case 'base_crop':
    content_url = this.compare.result.response.file1_crop;
    break;
  case 'base_comp_overlap':
    content_url = this.compare.result.response.file1_file2_overlap;
    break;
  case 'comp_crop_tx':
    content_url = this.compare.result.response.file2_crop_tx;
    break;
  case 'base_comp_diff':
    content_url = this.compare.result.response.file1_file2_diff;
    break;
  }
  return content_url;
}

_traherne_controller.prototype.show_element = function(e) {
  if( e.classList.contains('display-none') ) {
    e.classList.remove('display-none');
  }
  e.classList.add('display-inline-block');
}

_traherne_controller.prototype.hide_element = function(e) {
  if( e.classList.contains('display-inline-block') ) {
    e.classList.remove('display-inline-block');
  }
  e.classList.add('display-none');
}

///
/// toggle
///
// extract suffix from selector-id
// example: left_content_base_comp_diff => type=left_content, suffix=base_comp_diff
_traherne_controller.prototype.get_sid_suffix = function(type, sid) {
  var container_name = this.type_list[type];
  if( sid.startsWith(container_name) ) {
    return sid.substr( container_name.length + 1 );
  } else {
    console.log('get_sid_suffix(): mismatch ' + type + ', ' + sid);
    return '';
  }
}

_traherne_controller.prototype.get_container_type = function(id) {
  var container_type = '';
  for( var type in this.type_list ) {
    if( id.startsWith(this.type_list[type]) ) {
      container_type = type;
      break;
    }
  }
  return container_type;
}

_traherne_controller.prototype.toggle_event_handler = function(e) {
  var id = e.target.id;
  var type = this.get_container_type(id);
  if( e.target.checked ) {
    this.set_toggle(type);
  } else {
    this.clear_toggle(type);
  }
}

_traherne_controller.prototype.clear_toggle = function(type) {
  if( _traherne_toggle_timer[type] > 0 ) {
    clearInterval(_traherne_toggle_timer[type]);
    _traherne_toggle_timer[type] = 0;
    this.content_selector_group_set_state(type, true);

    // reset the content to that pointed by content selector
    var sid = this.get_current_content_selector_id(type);
    var sid_suffix = sid.substr( this.type_list[type].length + 1 );
    this.set_content(type, sid_suffix);
  }
}

_traherne_controller.prototype.get_current_content_selector_id = function(type) {
  var container = document.getElementById( this.type_list[type] + '_selector' );
  var child = container.getElementsByClassName('content_selector');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    if( child[i].checked ) {
      return child[i].id;
    }
  }
}

_traherne_controller.prototype.reset_all_toggle = function() {
  for( var type in this.type_list ) {
    // only reset the existing toggles
    if( _traherne_toggle_timer[type] > 0 ) {
      this.set_toggle(type);
    }
  }
}

_traherne_controller.prototype.set_toggle = function(type) {
  if( _traherne_toggle_timer[type] > 0 ) {
    this.clear_toggle(type);
  }

  var toggle_url_list = [];
  for( var t in this.type_list ) {
    var sid = this.get_current_content_selector_id(t);
    var sid_suffix = this.get_sid_suffix(t, sid);
    var url = this.get_content_url(t, sid_suffix);
    toggle_url_list.push(url);
  }

  if( type === 'comp' ) {
    // reverse the order
    toggle_url_list.reverse();
  }

  this.content_selector_group_set_state(type, false);
  _traherne_toggle_timer[type] = setInterval( function() {
    this.toggle_content(type, toggle_url_list);
  }.bind(this), this.v.theme.TOGGLE_SPEED);
}

_traherne_controller.prototype.toggle_content = function(type, toggle_url_list) {
  var container_name = this.type_list[type];
  var img_elem_name = container_name + '_image';
  var img_elem = document.getElementById(img_elem_name);

  var current_value = img_elem.getAttribute('src');
  var current_value_index = toggle_url_list.indexOf(current_value);

  if( current_value_index >= 0 ) {
    var next_index = current_value_index + 1;
    if ( next_index == (toggle_url_list.length) ) {
      next_index = 0;
    }
    img_elem.setAttribute('src', toggle_url_list[next_index]);

    // if zoom is enabled, toggle the zoom image as well
    if( this.v.now[type].zoom.is_enabled ) {
      var zoom_img = document.getElementById( this.type_list[type] + '_image_zoom' );
      zoom_img.setAttribute('src', toggle_url_list[next_index]);
    }
  } else {
    console.log('_traherne_controller.prototype.toggle_content : error');
    console.log(img_elem_name);
    console.log(toggle_url_list);
    return;
  }
}

_traherne_controller.prototype.enable_switch = function(type, switch_suffix) {
  var el = document.getElementById( this.type_list[type] + switch_suffix );
  if( el.hasAttribute('disabled') ) {
    el.removeAttribute('disabled');
    switch( switch_suffix ) {
    case '_toggle':
      el.addEventListener('click', this.toggle_event_handler.bind(this), false);
      break;
    case '_zoom':
      el.addEventListener('click', this.zoom_event_handler.bind(this), false);
      break;
    }
  }
}

_traherne_controller.prototype.disable_switch = function(type, switch_suffix) {
  var el = document.getElementById( this.type_list[type] + switch_suffix );
  if( !el.hasAttribute('disabled') ) {
    el.setAttribute('disabled', 'true');
    switch( switch_suffix ) {
    case '_toggle':
      el.removeEventListener('click', this.toggle_event_handler.bind(this), false);
      break;
    case '_zoom':
      el.removeEventListener('click', this.zoom_event_handler.bind(this), false);
      break;
    }
  }
}

_traherne_controller.prototype.disable_all_switches = function(switch_suffix) {
  for( var type in this.type_list ) {
    this.disable_switch(type, switch_suffix);
  }
}

_traherne_controller.prototype.enable_all_switches = function(switch_suffix) {
  for( var type in this.type_list ) {
    this.enable_switch(type, switch_suffix);
  }
}

///
/// horizontal reference line
///
_traherne_controller.prototype.ref_line_position = function(y) {
  var st = document.documentElement.scrollTop;
  var ot = document.getElementById('ref_line_container').offsetTop;
  //console.log('y=' + y + ', st=' + st + ', ot=' + ot + ', (ot + y - st)=' + (ot + y - st));
  this.ref_line.hline.style.top = (ot + y - st) + 'px';
  this.ref_line.current_y = y;
}

_traherne_controller.prototype.init_ref_line = function() {
  this.ref_line.hline = document.getElementById('horizontal_line');
  this.hide_element(this.ref_line.hline);
  this.ref_line.current_y = -1;

  window.addEventListener('scroll', function(e) {
    if( this.ref_line.hline.classList.contains('display-inline-block') ) {
      // update the ref_line
      this.ref_line_position(this.ref_line.current_y);
    }
  }.bind(this), false);

  var parent = document.getElementById('ref_line_container');
  parent.addEventListener('mouseup', function(e) {
    document.getElementById('href_line_toggler').checked = true;
    var y = e.offsetY;

    this.ref_line_position(y);
    this.show_element(this.ref_line.hline);
  }.bind(this), false);

  var hline = document.getElementById('horizontal_line');
  hline.addEventListener('mouseup', function(e) {
    this.hide_element(this.ref_line.hline);
    this.ref_line.current_y = -1;
    document.getElementById('href_line_toggler').checked = false;
  }.bind(this), false);

}


///
/// zoom
///
_traherne_controller.prototype.zoom_event_handler = function(e) {
  var id = e.target.id;
  var type = this.get_container_type(id);
  if( this.v.now[type].sid_suffix.endsWith('_via') ) {
    if( e.target.checked ) {
      this.v.now[type].zoom.is_enabled = true;
      this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
      this.m.via[type].c.zoom_activate();
    } else {
      this.v.now[type].zoom.is_enabled = false;
      this.m.via[type].c.zoom_deactivate();
    }
  } else {
    if( e.target.checked ) {
      // enable image zoom
      this.enable_image_zoom(type);
    } else {
      this.disable_image_zoom(type);
    }
  }
}

_traherne_controller.prototype.zoom_update_level = function() {
  for( var type in this.type_list ) {
    // ensure that files are loaded for "type"
    if ( typeof(this.v.now[type].findex) !== 'undefined' ) {
      var switch_name = this.type_list[type] + '_zoom';
      var e = document.getElementById(switch_name);

      if( this.v.now[type].sid_suffix.endsWith('_via') ) {
        if( e.checked ) {
          this.v.now[type].zoom.is_enabled = false;
          this.m.via[type].c.zoom_deactivate();
          this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
          this.m.via[type].c.zoom_activate();
          this.v.now[type].zoom.is_enabled = true;
        }
      } else {
        if( e.checked ) {
          this.v.now[type].zoom.is_enabled = false;
          this.disable_image_zoom(type);
          this.enable_image_zoom(type);
          this.v.now[type].zoom.is_enabled = true;
        }
      }
    }
  }
}


_traherne_controller.prototype.disable_image_zoom = function(type) {
  this.v.now[type].zoom.is_enabled = false;
  this.v.now[type].zoom.is_frozen = false;

  var top_panel = document.getElementById( this.type_list[type] + '_image_top_panel' );
  top_panel.removeEventListener('mousemove', this.v.now[type].zoom.mousemove_el, false);
  top_panel.removeEventListener('mouseout', this.v.now[type].zoom.mouseout_el, false);
  top_panel.removeEventListener('mousedown', this.v.now[type].zoom.mousedown_el, false);

  var zoom = document.getElementById( this.type_list[type] + '_image_zoom_panel' );
  zoom.setAttribute('style', '');
  zoom.innerHTML = '';
  this.hide_element(zoom);
  this.hide_element(top_panel);
}

_traherne_controller.prototype.enable_image_zoom = function(type) {
  this.v.now[type].zoom.is_enabled = true;
  this.v.now[type].zoom.is_frozen = false;
  var img0 = document.getElementById( this.type_list[type] + '_image' );

  var zoom_img = document.createElement('img');
  zoom_img.setAttribute('id', this.type_list[type] + '_image_zoom')
  zoom_img.setAttribute('src', img0.getAttribute('src'));
  zoom_img.setAttribute('width', img0.width * this.v.theme.ZOOM_LEVEL + 'px');
  zoom_img.setAttribute('height', img0.height * this.v.theme.ZOOM_LEVEL + 'px');
  var zoom = document.getElementById( this.type_list[type] + '_image_zoom_panel' );
  this.show_element(zoom);
  zoom.setAttribute('style', '');
  zoom.appendChild(zoom_img);

  var top_panel = document.getElementById( this.type_list[type] + '_image_top_panel' );
  this.show_element(top_panel);

  // this is needed as bind() creates a new function reference
  // see https://stackoverflow.com/a/22870717/7814484
  this.v.now[type].zoom.mousemove_el = this.image_zoom_mousemove_handler.bind(this);
  this.v.now[type].zoom.mouseout_el = this.image_zoom_mouseout_handler.bind(this);
  this.v.now[type].zoom.mousedown_el = this.image_zoom_mousedown_handler.bind(this);
  top_panel.addEventListener('mousemove', this.v.now[type].zoom.mousemove_el, false);
  top_panel.addEventListener('mouseout', this.v.now[type].zoom.mouseout_el, false);
  top_panel.addEventListener('mousedown', this.v.now[type].zoom.mousedown_el, false);

  this.show_message('To <span class="blue">freeze the zoom region</span>, click the left mouse button when zoom is active. Click the left mouse button again to unfreeze the zoom region.');
}

_traherne_controller.prototype.image_zoom_mousedown_handler = function(e) {
  var type = this.get_container_type(e.currentTarget.id);
  var top_panel = e.currentTarget;
  if( this.v.now[type].zoom.is_frozen ) {
    this.v.now[type].zoom.is_frozen = false;
    this.v.now[type].zoom.mousemove_el = this.image_zoom_mousemove_handler.bind(this);
    top_panel.addEventListener('mousemove', this.v.now[type].zoom.mousemove_el, false);
  } else {
    this.v.now[type].zoom.is_frozen = true;
    top_panel.removeEventListener('mousemove', this.v.now[type].zoom.mousemove_el, false);
  }
}

_traherne_controller.prototype.image_zoom_mouseout_handler = function(e) {
  var type = this.get_container_type(e.currentTarget.id);
  if( ! this.v.now[type].zoom.is_frozen ) {
    var content_prefix = e.currentTarget.id.replace('_top_panel', '');
    var zoom = document.getElementById( content_prefix + '_zoom_panel' );
    zoom.setAttribute('style', 'width:0; height:0;');
  }
}

_traherne_controller.prototype.image_zoom_mousemove_handler = function(e) {
  var content_prefix = e.currentTarget.id.replace('_top_panel', '');
  var px = e.offsetX;
  var py = e.offsetY;
  //console.log('content_prefix='+content_prefix+', x='+px+', y='+py);

  // set zoomed image location
  var img1_top = this.v.theme.ZOOM_WINDOW_SIZE_BY2 - ( py * this.v.theme.ZOOM_LEVEL);
  var img1_left = this.v.theme.ZOOM_WINDOW_SIZE_BY2 - ( px * this.v.theme.ZOOM_LEVEL);
  var img1_style = 'left:' + img1_left + 'px' + ';top:' + img1_top + 'px';
  var img1 = document.getElementById( content_prefix + '_zoom' );
  img1.setAttribute('style', img1_style);

  // setup zoom panel (circular magnifying glass)
  var zoom_panel_left = px - this.v.theme.ZOOM_WINDOW_SIZE_BY2;
  var zoom_panel_top = py - this.v.theme.ZOOM_WINDOW_SIZE_BY2;
  var zp_style = [];
  zp_style.push('width: ' + this.v.theme.ZOOM_WINDOW_SIZE + 'px');
  zp_style.push('height: ' + this.v.theme.ZOOM_WINDOW_SIZE + 'px');
  zp_style.push('top: ' + zoom_panel_top + 'px');
  zp_style.push('left: ' + zoom_panel_left + 'px');
  zp_style.push('border-radius: ' + this.v.theme.ZOOM_WINDOW_SIZE_BY2 + 'px');
  var zoom = document.getElementById( content_prefix + '_zoom_panel' );
  zoom.setAttribute('style', zp_style.join(';'));
}

///
/// Download of current visualisation
///
_traherne_controller.prototype.save_current_visualisation = function() {
  console.log('save @todo')
}

///
/// Image Rotation
///
_traherne_controller.prototype.transform_remote_file = function(d) {

  var tokens = d.split('_');
  var type = tokens[0];
  var operation = tokens[1];
  var param = tokens[2];
  var findex = this.v.now[type].findex;
  var via_fid = this.m.index_to_fid[type][findex];
  var sid_suffix = type + '_via';

  // image rotation is only allowed for full image
  if ( this.v.now[type].sid_suffix !== sid_suffix ) {
    this.show_message('Image rotation and flip operations are <span class="red">only allowed</span> for full images of base and comp.');
    return;
  }

  this.m.upload[type][findex].then( function(result) {
    if ( !result.success ) {
      this.show_message('Cannot ' + operation + ' because file ' +
                        '[ ' + this.m.files[result1.type][result1.findex].name + ' ] ' +
                        '<span class="red">failed to upload</span>.');
      return;

    }
    var remote_fid = result.fid;
    var transform_promise = this.m.transform_remote_file(type, findex, via_fid, remote_fid, operation, param);
    transform_promise.then( function(ok) {
      // to force, web browser to download a new copy of an existing uri,
      // we add a random number to the remote file uri
      var uri = ok.uri + '&force_img_fetch=' + Date.now();
      this.m.via[ok.type].m.files.content[ok.via_fid] = uri;
      this.m.via[ok.type].m.files.metadata[ok.via_fid].source = 'url'
      this.m.files[ok.type][ok.findex].location = 'remote';
      this.m.files[ok.type][ok.findex].uri = uri;
      if ( this.m.via[ok.type].v.now.all_rid_list.length ) {
        this.m.via[ok.type].c.region_delete( this.m.via[ok.type].v.now.all_rid_list ); // delete all existing regions
      }
      this.m.via[ok.type].c.load_file(ok.via_fid);
    }.bind(this), function(err) {
      console.log(err);
      this.show_message('Cannot ' + operation + ' because file ' +
                        '[ ' + this.m.files[err.type][err.findex].name + ' ] ' +
                        '<span class="red">' + err.msg + '</span>.');
    }.bind(this));
  }.bind(this));
}

_traherne_controller.prototype.rotate_remote_file = function(type, findex, via_fid, operation, param) {
  return new Promise( function(ok_callback, err_callback) {

  }.bind(this));
}

///
/// messages
///
_traherne_controller.prototype.show_message = function(msg) {
  document.getElementById('message_panel').innerHTML = msg;
}

_traherne_controller.prototype.clear_message = function() {
  document.getElementById('message_panel').innerHTML = '&nbsp;';
}

_traherne_controller.prototype.append_message = function(msg) {
  document.getElementById('message_panel').innerHTML += msg;
}
