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
// @file        _imcomp_controller.js
// @description Controller of the MVC design for user interface javascript
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        July 2018
//
////////////////////////////////////////////////////////////////////////////////

function _imcomp_controller() {
  this.type_list = {'base':'left_content', 'comp':'right_content'};
  this.type_description = {'base':'Base', 'comp':'Comp.'};

  this.state = {};

  this.config = {};
  this.config.upload = {};
  this.config.upload.MAX_IMG_DIM_PX = 1200;
  this.config.imcomp_server_upload_uri = "/imcomp/_upload";
  this.config.imcomp_server_compare_uri = "/imcomp/_compare";
  this.config.imcomp_cache_uri = "/imcomp/_cache";

  this.compare = {};
  this.compare.is_ongoing = false;
  this.compare.start_time = {};
  this.compare.end_time = {};
  this.compare.promise = {};  // promise to current compare operation
  this.compare.result = {};
  this.compare.algorithm = {};

  this.ref_line = {};

  // everything to do with results visualization
  this.results = {};
  this.results.is_hor_slider_pressed  = false;
  this.results.is_vert_slider_pressed = false;
  this.results.active_tab = 'default';
  this.results.canvas_width = 0;
  this.results.canvas_height = 0;

  // eveything to do with the toolbar on top
  this.toolbar = {};
  this.toolbar.zoom_enabled = false;
  this.toolbar.current_page = 'home'; // home | compare | result
}

function _imcomp_compare_instance(findex1, findex2) {
  this.findex1 = findex1;
  this.findex2 = findex2;
}

_imcomp_controller.prototype.init = function( imcomp_model, imcomp_view ) {
  this.m = imcomp_model;
  this.v = imcomp_view;

  for(var type in this.type_list) {
    this.clear_content(type);
  }
  this.disable_all_content_selectors();
  this.disable_all_switches('_toggle');
  this.disable_all_switches('_zoom');
  this.init_ref_line();

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

_imcomp_controller.prototype.reset_controller_state = function(type) {
  this.m.reset_model_state(type);
  this.clear_content(type);

  this.content_selector_group_set_state(type, false);
  this.disable_switch(type, '_toggle');
  this.disable_switch(type, '_zoom');
}

_imcomp_controller.prototype.update_files = function(e) {
  //this.reset_controller_state(type);
  this.m.add_images(e.target.files);
}

_imcomp_controller.prototype.update_dropped_files = function(e) {
  this.m.add_images(e.dataTransfer.files);
}

_imcomp_controller.prototype.file_dropped = function(img_elem_id, type) {
  var file_idx = img_elem_id.replace('files_panel_file_', '');
  if ( type === 'base' ) {
    this.set_now('base', file_idx);
  }
  if ( type === 'comp' ) {
    this.set_now('comp', file_idx);
    show_instruction("Click compare to compare both the images. You may also draw a region on the left image to compare image regions.<br/> Optionally you may select a transform before clicking compare.");
  }

  // make an async request to pre-compute features and cache them
  // this.m.compute_cache_features(file_idx);
}

_imcomp_controller.prototype.move_to_next = function(type) {
  var n = this.m.files.length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === (n - 1) ) {
      this.set_now(type, 0);
    } else {
      this.set_now(type, now_findex + 1);
    }
    this.disable_all_content_selectors();
  }
}

_imcomp_controller.prototype.move_to_prev = function(type) {
  var n = this.m.files.length;
  if( n > 0 ) {
    var now_findex = this.v.now[type].findex;
    if( now_findex === 0 ) {
      this.set_now(type, n - 1);
    } else {
      this.set_now(type, now_findex - 1);
    }
    this.disable_all_content_selectors();
  }
}

_imcomp_controller.prototype.move_to_next_pair = function() {
  for( var type in this.type_list ) {
    this.move_to_next(type);
  }
}

_imcomp_controller.prototype.move_to_prev_pair = function() {
  for( var type in this.type_list ) {
    this.move_to_prev(type);
  }
}

_imcomp_controller.prototype.on_filelist_update = function() {
  var p = document.getElementById('step1_file_added_count');
  p.innerHTML = this.m.file_count + ' files uploaded';
  if ( this.m.file_count < 2 ) {
    p.innerHTML += '  <span style="color:red;">(To compare, you must add at least 2 files)</span>';
  }

  if ( this.m.file_count > 1 ) {
    document.getElementById('step1_files_added_nav').classList.remove('display-none');
  }
  // show_message('Added [' + this.m.file_count + '] files. Drag and drop images from top to compare.');

  _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP3, false);
}

_imcomp_controller.prototype.update_view_filelist = function(type) {
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

_imcomp_controller.prototype.set_now = function(type, findex) {
  this.m.via[type].c.load_file_from_index( findex );
  this.v.now[type].findex = findex;

  // update the content to show {base,comp} full image
  var sid_suffix = type + '_via';
  this.content_selector_set_state(type, sid_suffix, true);
  this.set_content(type, sid_suffix);

  this.on_now_update(type);
}

_imcomp_controller.prototype.on_now_update = function(type) {
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
}

_imcomp_controller.prototype.update_now = function(type, e) {
  var findex = parseInt(e.target.value, 10);
  this.set_now(type, findex);
}

_imcomp_controller.prototype.on_upload_status_update = function(findex) {
  if ( this.v.now.findex == findex ) {
    this.update_now_file_status(type);
  }
}

_imcomp_controller.prototype.update_now_file_status = function(type) {
  var now_findex = this.v.now[type].findex;
  console.log(this.m.upload_status);
  console.log(now_findex);
  console.log(type)
  var status = this.m.upload_status[now_findex].status;
  var msg = this.m.upload_status[now_findex].msg;
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

_imcomp_controller.prototype.is_upload_resize_req = function(findex, w, h) {
  if( w > this.config.upload.MAX_IMG_DIM_PX || h > this.config.upload.MAX_IMG_DIM_PX ) {
    return true;
  } else {
    return false;
  }
}

///
/// Comparison of Base and Comp. pair
///
_imcomp_controller.prototype.compare_base_comp = function() {
  console.log('comparing');
  // sanity checks
  for( type in this.type_list ) {
    if( this.m.file_count[type] === 0 ) {
      var type_name = this.type_list[type];
      return;
    }
  }

  if( this.compare.is_ongoing ) {
    show_message('Please wait, compare process is <span class="blue">ongoing</span> ...');
    return;
  }


  if( !this.is_base_region_selected() ) {
    show_message('Comparing images as no regions were selected.');
    // choose the entire image as a region and compare
    var x0 = 0;
    var y0 = 0;
    // var x1 = this.m.via['base'].v.now.tform.content_width;
    // var y1 = this.m.via['base'].v.now.tform.content_height;
    var x1 = this.m.via['base'].v.now.tform.width;
    var y1 = this.m.via['base'].v.now.tform.height;
    var via = this.m.via['base'];
    // create new points for mouse down and up
    via.v.last.mousedown = new _via_point(x0, y0);
    via.v.nvertex.push( x0 );
    via.v.nvertex.push( y0 );
    via.v.last.mouseup = new _via_point(x1, y1);
    via.v.nvertex.push( x1 );
    via.v.nvertex.push( y1 );
    via.c.add_region_from_nvertex();
    via.v.nvertex.splice(0);
  }

  var findex1 = this.v.now['base'].findex;
  var findex2 = this.v.now['comp'].findex;
  var c = new _imcomp_compare_instance(findex1, findex2);
  this.m.upload[findex1].then( function(upload_id1) {
    c.upload_id1 = upload_id1;
    c.scale1 = this.m.upload_scale[c.findex1];
    var via_fid1 = this.m.fid_to_via_fileid['base'][c.findex1];
    var rid = this.m.via['base'].v.now.all_rid_list[0];
    c.region1 = this.m.via['base'].m.regions[via_fid1][rid].dimg.slice(0);
    this.m.upload[findex2].then( function(upload_id2) {
      c.upload_id2 = upload_id2;
      c.scale2 = this.m.upload_scale[c.findex2];
      this.compare.is_ongoing = true;
      this.compare.promise = this.m.compare_img_pair(c);

      if ( !this.is_base_region_selected() ) {
        show_message('Comparing selected region in images... Please wait. (Larger regions mean longer time)');
      } else {
        show_message('Comparing images... Please wait. (Larger images mean longer time)');
      }

    }.bind(this));
  }.bind(this));
}

_imcomp_controller.prototype.is_base_region_selected = function() {
  if ( this.m.via['base'].v.now.all_rid_list.length == 1 ) {
    return true;
  } else {
    return false;
  }
}

_imcomp_controller.prototype.on_compare_start = function() {
  this.compare.start_time = new Date();
  document.getElementById('compare_base_comp').disabled = true;
}

_imcomp_controller.prototype.on_compare_end = function() {
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

_imcomp_controller.prototype.on_compare_status_update = function() {
  show_message('Compare status: ' + this.m.compare_status.msg);
}

_imcomp_controller.prototype.on_compare_success = function() {
  // note: this.compare.result contains the result
  var time = this.compare.end_time - this.compare.start_time;
  show_message('Comparison <span class="blue">completed in ' + Math.round(time/1000) + ' sec.</span>');

  this.enable_results_tabs();
  this.format_results_page();
  this.enable_all_content_selectors();
  this.toolbar.current_page = 'result';
  // store the width and height for resetting zoom
  var img_div = document.getElementById('left_content_image');
  this.results.canvas_width = img_div.style.width;
  this.results.canvas_height = img_div.style.width;
}

_imcomp_controller.prototype.format_comparison_page = function() {
	// make sure items we disable are visible
	document.getElementById('ref_line_container').style.display = '';
	document.getElementById('right_content').style.display = '';
  document.getElementById('files_panel').style.display = '';
  document.getElementById('toggle_controls').classList.add('display-none');
  document.getElementById('fade_controls').classList.add('display-none');
	document.getElementById('results_tabs_panel').style.display = 'none';

  document.getElementById('top_panel').classList.remove('display-none');
  document.getElementById('banner').style.display = 'none';

  document.getElementById('instructions_panel').style.display = 'block';
  show_instruction("Select images by dragging them from the Files Panel and dropping them on the View Panel.");

  document.getElementById('left_content_container').style.height = '66vh';
  document.getElementById('left_content_container').classList.add('imcomp_border');

  document.getElementById('tab_tools_panel').classList.add('display-none');
}

_imcomp_controller.prototype.format_results_page = function() {
  // remove files panel, compare buttons panel and add results tabs to show results
  document.getElementById('files_panel').style.display = 'none';
  document.getElementById('results_tabs_panel').style.display = 'block';
  document.getElementById('right_content').style.display = 'none';
  document.getElementById('ref_line_container').style.display = 'none';
  document.getElementById('left_content').title = "Result of your comparison";
  this.brighten_instructions('step4');
  var cp_header = document.getElementsByClassName('contents_panel_header')[0];
  cp_header.classList.add('display-none');
  var fp_header = document.getElementsByClassName('files_panel_header')[0];
  fp_header.classList.add('display-none');
  // default settings
  this.set_content('base', 'base_crop');
  this.set_toggle('base');
  this.deactivate_results_tabs();
  document.getElementById('results_tabs_default').classList.add('active');
  document.getElementById('left_content_container').style.height = 'auto';

  document.getElementById('top_panel').classList.remove('display-none');
  document.getElementById('banner').classList.add('display-none');
  show_instruction("Visualize results in different ways by clicking on the tabs below. <br/> You may also zoom in, zoom out and magnify regions using the toolbar on the top left of the page.");

  var base_img = document.getElementById('left_content_image');
  console.log('img width is: ', base_img.width);
  console.log('img height is: ', base_img.height);

  document.getElementById('left_content_container').classList.remove('imcomp_border');
  document.getElementById('tab_tools_panel').classList.remove('display-none');

  // document.getElementById('left_content_container').style.overflowY = 'scroll';
  // base_img.style.height = '600px';
  // base_img.style.height = Math.round(base_img.offsetHeight * 0.9) + 'px';
  // var slide_img = document.getElementById('hor_slide_overlay_img');
  // slide_img.style.width = Math.round(slide_img.offsetWidth * 0.8) + 'px';
  // slide_img.style.height = Math.round(slide_img.offsetHeight * 0.8) + 'px';
}

_imcomp_controller.prototype.enable_results_tabs = function() {
  var rt = document.getElementById('results_tabs_panel');
  var tabs = rt.getElementsByClassName('results_tab');
  for (var i=0; i < tabs.length; i++ ) {
    tabs[i].addEventListener('click', this.results_tab_event_handler.bind(this), false);
  }
}

_imcomp_controller.prototype.brighten_instructions = function(panel_id) {
  switch (panel_id) {
    case 'step1':
      document.getElementById('instruction_step1').classList.add('instruction_active');
      document.getElementById('instruction_step2').classList.remove('instruction_active');
      document.getElementById('instruction_step3').classList.remove('instruction_active');
      break;
    case 'step3':
      document.getElementById('instruction_step1').classList.remove('instruction_active');
      document.getElementById('instruction_step2').classList.add('instruction_active');
      document.getElementById('instruction_step3').classList.remove('instruction_active');
      break;
    case 'step4':
      document.getElementById('instruction_step1').classList.remove('instruction_active');
      document.getElementById('instruction_step2').classList.remove('instruction_active');
      document.getElementById('instruction_step3').classList.add('instruction_active');
      break;
    default:
  }
}

_imcomp_controller.prototype.results_tab_event_handler = function(e) {
  e.stopPropagation();
  this.deactivate_results_tabs();
  e.target.classList.add('active');
  // always display the result in base
  var container_type = 'base';
  var target_id = e.target.id;
  this.results.active_tab = target_id.replace('results_tabs_', '');

  // display tab specific tools
  switch ( target_id ) {
    case 'results_tabs_default':
      document.getElementById('slide_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('fade_controls').classList.add('display-none');
      // toggle at medium speed by default
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
      this.set_toggle('base');
      this.remove_slider_elem('all');
      this.remove_overlay_elem();
      break;

    case 'results_tabs_overlay':
      document.getElementById('slide_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('fade_controls').classList.add('display-none');
      var slide_elem = document.getElementById('base_comp_fader');
      document.getElementById('base_comp_fader').value = 50;
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
      this.clear_toggle('base');
      this.remove_overlay_elem();
      this.remove_slider_elem('all');
      // fader at 50% is overlay
      this.add_fader_overlay();
      // reset any zoom done before
      var base_img = document.getElementById('left_content_image');
      base_img.style.width = this.results.canvas_width;
      base_img.style.height = this.results.canvas_height;
      break;

    case 'results_tabs_toggle':
      document.getElementById('fade_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.remove('display-none');
      document.getElementById('slide_controls').classList.add('display-none');
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
      this.set_toggle('base');
      this.remove_slider_elem('all');
      this.remove_overlay_elem();
      break;

    case 'results_tabs_fade':
      document.getElementById('fade_controls').classList.remove('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('slide_controls').classList.add('display-none');
      var slide_elem = document.getElementById('base_comp_fader');
      document.getElementById('base_comp_fader').value = 50;
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
      this.clear_toggle('base');
      this.remove_overlay_elem();
      this.remove_slider_elem('all');
      this.add_fader_overlay();
      // reset any zoom done before
      var base_img = document.getElementById('left_content_image');
      base_img.style.width = this.results.canvas_width;
      base_img.style.height = this.results.canvas_height;
      break;

    case 'results_tabs_slide':
      document.getElementById('fade_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('slide_controls').classList.remove('display-none');
      document.getElementById('slide_radio_horizontal').checked = true;
      this.clear_toggle('base');
      this.add_hor_slider_overlay();
      // this.add_vert_slider_overlay();
      // reset any zoom done before
      var base_img = document.getElementById('left_content_image');
      base_img.style.width = this.results.canvas_width;
      base_img.style.height = this.results.canvas_height;
      e.stopPropagation();
      break;

    case 'results_tabs_hover':
      document.getElementById('fade_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('slide_controls').classList.add('display-none');
      this.clear_toggle('base');
      this.remove_overlay_elem();
      this.remove_slider_elem('all');
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
      break;

      default: // overlay and difference views
      document.getElementById('fade_controls').classList.add('display-none');
      document.getElementById('toggle_controls').classList.add('display-none');
      document.getElementById('slide_controls').classList.add('display-none');
      this.clear_toggle('base');
      this.remove_overlay_elem();
      this.remove_slider_elem('all');
      // choose suffix as per traherne
      var sid_suffix = this.get_sid_suffix_for_results_tab(e);
      this.set_content(container_type, sid_suffix);
  }
}

_imcomp_controller.prototype.add_fader_overlay = function() {
  var base_div = document.getElementById('left_content_img_panel');
  base_div.classList.add('img-comp-img');
  base_div.style.position = 'absolute';

  var overlay_div = document.getElementById('hor_slide_overlay');
  overlay_div.classList.add('display-inline-block');
  overlay_div.style.width = base_div.offsetWidth + "px";

  var overlay_img = document.getElementById('hor_slide_overlay_img');
  overlay_img.style.opacity = 0.5;
  overlay_img.src = this.get_content_url('comp', 'comp_crop_tx');
  // reset any slide done before
  overlay_img.style.width = this.results.canvas_width;
  overlay_img.style.height = this.results.canvas_height;
}

_imcomp_controller.prototype.remove_overlay_elem = function() {
  var hor_overlay_div = document.getElementById('hor_slide_overlay');
  var vert_overlay_div = document.getElementById('vert_slide_overlay');
  hor_overlay_div.classList.remove('display-inline-block');
  hor_overlay_div.classList.add('display-none');
  vert_overlay_div.classList.remove('display-inline-block');
  vert_overlay_div.classList.add('display-none');
}

_imcomp_controller.prototype.remove_slider_elem = function (elem) {
  var hor_slider = document.getElementById('horizontal_slider');
  var vert_slider = document.getElementById('vertical_slider');
  if (elem === 'horizontal') {
    hor_slider.classList.add('display-none');
  }
  if (elem === 'vertical') {
    vert_slider.classList.add('display-none');
  }
  if (elem === 'all') {
    hor_slider.classList.add('display-none');
    vert_slider.classList.add('display-none');
  }
}

_imcomp_controller.prototype.add_hor_slider_overlay = function () {
  var img_overlay = document.getElementById('hor_slide_overlay_img');
  img_overlay.src = this.get_content_url('comp', 'comp_crop_tx');
  img_overlay.style.opacity = 1.0;

  var left_img = document.getElementById('left_content_image');
  left_img.src = this.get_content_url('base', 'base_crop');

  var base_div = document.getElementById('left_content_img_panel');
  var w = base_div.offsetWidth;
  var h = base_div.offsetHeight;
  base_div.classList.add('img-comp-img');
  base_div.style.position = 'absolute';

  var overlay_div = document.getElementById('hor_slide_overlay');
  overlay_div.classList.add('display-inline-block');
  overlay_div.style.width = (w / 2) + "px";

  // position slider and bind its actions
  var slider = document.getElementById('horizontal_slider');
  slider.classList.remove('display-none');
  slider.style.top = (h / 2) - (slider.offsetHeight / 2) + "px";
  slider.style.left = (w / 2) - (slider.offsetWidth / 2) + "px";
}

_imcomp_controller.prototype.add_vert_slider_overlay = function () {
  var img_overlay = document.getElementById('vert_slide_overlay_img');
  img_overlay.src = this.get_content_url('comp', 'comp_crop_tx');
  img_overlay.style.opacity = 1.0;

  var left_img = document.getElementById('left_content_image');
  left_img.src = this.get_content_url('base', 'base_crop');

  var base_div = document.getElementById('left_content_img_panel');
  var w = base_div.offsetWidth;
  var h = base_div.offsetHeight;
  base_div.classList.add('img-comp-img');
  base_div.style.position = 'absolute';

  var overlay_div = document.getElementById('vert_slide_overlay');
  overlay_div.classList.add('display-inline-block');
  overlay_div.style.height = (h / 2) + "px";

  // position slider and bind its actions
  var slider = document.getElementById('vertical_slider');
  slider.classList.remove('display-none');
  slider.style.top = (h / 2) - (slider.offsetHeight / 2) + "px";
  slider.style.left = (w / 2) - (slider.offsetWidth / 2) + "px";
}

_imcomp_controller.prototype.deactivate_results_tabs = function () {
  var rt = document.getElementById('results_tabs_panel');
  var tabs = rt.getElementsByClassName('results_tab');
  for (var i=0; i < tabs.length; i++ ) {
    tabs[i].classList.remove('active');
  }
}

_imcomp_controller.prototype.get_sid_suffix_for_results_tab = function(e) {
  var id = e.target.id;
  var sid_suffix;
  switch ( id ) {
  case 'results_tabs_default':
    sid_suffix = 'base_crop';
    break;
  case 'results_tabs_diff':
    sid_suffix = 'base_comp_diff';
    break;
  // overlay is same as fade with slider set at 50%
  case 'results_tabs_overlay':
  case 'results_tabs_fade':
    sid_suffix = 'base_crop';
    break;
  default:
    sid_suffix = 'base_crop';
    break;
  }
  return sid_suffix;
}

_imcomp_controller.prototype.on_compare_failure = function() {
  var time = this.compare.end_time - this.compare.start_time;
  show_message('<span class="red">Comparison failed!</span');
}

_imcomp_controller.prototype.show_compare_result = function(c) {
  // NOTE: this.compare.result contains the comparison result
  // var c = this.compare.result.response;
  // c = {"homography", "file1_crop", "file2_crop", "file2_crop_tx", "file1_file2_diff"}
  var c = this.compare.result.response;
  // in base panel, show file1 crop
  this.set_content('base', 'base_crop');

  // in comp panel, show file2 transformed + crop
  this.set_content('comp', 'comp_crop_tx');
}

_imcomp_controller.prototype.disable_all_content_selectors = function() {
  for( var type in this.type_list ) {
    this.content_selector_group_set_state(type, false);
  }
}

_imcomp_controller.prototype.enable_all_content_selectors = function() {
  for( var type in this.type_list ) {
    this.content_selector_group_set_state(type, true);
  }
}

_imcomp_controller.prototype.content_selector_set_state = function(type, sid_suffix, is_enabled) {
  var container_name_prefix = this.type_list[type];
  var content_selector_name = container_name_prefix + '_' + sid_suffix;
  var content_selector = document.getElementById(content_selector_name);
  if(is_enabled) {
    this.enable_content_selector(content_selector);
  } else {
    this.disable_content_selector(content_selector);
  }
}

_imcomp_controller.prototype.content_selector_set_checked = function(type, name_suffix) {
  this.content_selector_uncheck_all(type);
  var container_name_prefix = this.type_list[type];
  var content_selector_name = container_name_prefix + '_' + name_suffix;
  var content_selector = document.getElementById(content_selector_name);
  content_selector.checked = true;
}

_imcomp_controller.prototype.content_selector_uncheck_all = function(type) {
  var container_name = this.type_list[type] + '_selector';
  var container = document.getElementById(container_name);
  var child = container.getElementsByClassName('content_selector');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    child[i].checked = false;
  }
}

_imcomp_controller.prototype.enable_content_selector = function(element) {
  element.removeAttribute('disabled');
  element.addEventListener('click', this.content_selector_event_handler.bind(this), false);
}

_imcomp_controller.prototype.disable_content_selector = function(element) {
  element.setAttribute('disabled', 'disabled');
  element.removeEventListener('click', this.content_selector_event_handler.bind(this), false);
}

_imcomp_controller.prototype.content_selector_group_set_state = function(type, is_enabled) {
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

_imcomp_controller.prototype.content_selector_event_handler = function(e) {
  var id = e.target.id;
  var container_type = this.get_container_type(id);
  var container_name = this.type_list[container_type];
  var sid_suffix = id.substr(container_name.length + 1);
  this.set_content(container_type, sid_suffix);
}

_imcomp_controller.prototype.clear_content = function(type) {
  var container_name = this.type_list[type] + '_container';
  var container = document.getElementById(container_name);
  var child = container.getElementsByClassName('content');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    this.hide_element(child[i]);
  }
  this.disable_image_zoom(type);
}

_imcomp_controller.prototype.set_content = function(type, sid_suffix) {
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
      show_message('In the difference image, <span style="color: blue">blue</span> is for left image and <span style="color: red">red</span> is for right image');
    }
  }
  this.enable_switch(type, '_zoom');
  // this.content_selector_set_checked(type, sid_suffix);
  this.v.now[type].sid_suffix = sid_suffix;
}

_imcomp_controller.prototype.get_content_url = function(type, sid_suffix) {
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

_imcomp_controller.prototype.show_element = function(e) {
  if( e.classList.contains('display-none') ) {
    e.classList.remove('display-none');
  }
  e.classList.add('display-inline-block');
}

_imcomp_controller.prototype.hide_element = function(e) {
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
_imcomp_controller.prototype.get_sid_suffix = function(type, sid) {
  var container_name = this.type_list[type];
  if( sid.startsWith(container_name) ) {
    return sid.substr( container_name.length + 1 );
  } else {
    console.log('get_sid_suffix(): mismatch ' + type + ', ' + sid);
    return '';
  }
}

_imcomp_controller.prototype.get_container_type = function(id) {
  var container_type = '';
  for( var type in this.type_list ) {
    if( id.startsWith(this.type_list[type]) ) {
      container_type = type;
      break;
    }
  }
  return container_type;
}

_imcomp_controller.prototype.toggle_event_handler = function(e) {
  var id = e.target.id;
  var type = this.get_container_type(id);
  if( e.target.checked ) {
    this.set_toggle(type);
  } else {
    this.clear_toggle(type);
  }
}

_imcomp_controller.prototype.clear_toggle = function(type) {
  if( _imcomp_toggle_timer[type] > 0 ) {
    clearInterval(_imcomp_toggle_timer[type]);
    _imcomp_toggle_timer[type] = 0;
    this.content_selector_group_set_state(type, true);

    // reset the content to that pointed by content selector
    var sid = this.get_current_content_selector_id(type);
    // var sid_suffix = sid.substr( this.type_list[type].length + 1 );
    sid_suffix = 'base_crop';
    this.set_content(type, sid_suffix);
  }
}

_imcomp_controller.prototype.get_current_content_selector_id = function(type) {
  var container = document.getElementById( this.type_list[type] + '_selector' );
  var child = container.getElementsByClassName('content_selector');
  var n = child.length;
  for( var i=0; i<n; i++ ) {
    if( child[i].checked ) {
      return child[i].id;
    }
  }
}

_imcomp_controller.prototype.set_toggle = function(type) {
  if( _imcomp_toggle_timer[type] > 0 ) {
    this.clear_toggle(type);
  }

  var toggle_url_list = [];
  var sid_suffixes = ['base_crop', 'base_comp_overlap'];
  var k = 0;
  for( var t in this.type_list ) {
    // var sid = this.get_current_content_selector_id(t);
    // var sid_suffix = this.get_sid_suffix(t, sid);
    var url = this.get_content_url(t, sid_suffixes[k]);
    toggle_url_list.push(url);
    k += 1;
  }

  if( type === 'comp' ) {
    // reverse the order
    toggle_url_list.reverse();
  }

  this.content_selector_group_set_state(type, false);
  _imcomp_toggle_timer[type] = setInterval( function() {
    this.toggle_content(type, toggle_url_list);
  }.bind(this), this.v.theme.TOGGLE_SPEED);
}

_imcomp_controller.prototype.reset_all_toggle = function() {
  for( var type in this.type_list ) {
    // only reset the existing toggles
    if( _imcomp_toggle_timer[type] > 0 ) {
      this.set_toggle(type);
    }
  }
}

_imcomp_controller.prototype.toggle_content = function(type, toggle_url_list) {
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
    console.log('_imcomp_controller.prototype.toggle_content : error');
    console.log(img_elem_name);
    console.log(toggle_url_list);
    return;
  }
}

_imcomp_controller.prototype.enable_switch = function(type, switch_suffix) {
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

_imcomp_controller.prototype.disable_switch = function(type, switch_suffix) {
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

_imcomp_controller.prototype.disable_all_switches = function(switch_suffix) {
  for( var type in this.type_list ) {
    this.disable_switch(type, switch_suffix);
  }
}

_imcomp_controller.prototype.enable_all_switches = function(switch_suffix) {
  for( var type in this.type_list ) {
    this.enable_switch(type, switch_suffix);
  }
}

///
/// horizontal reference line
///
_imcomp_controller.prototype.ref_line_position = function(y) {
  var st = document.documentElement.scrollTop;
  var ot = document.getElementById('ref_line_container').offsetTop;
  //console.log('y=' + y + ', st=' + st + ', ot=' + ot + ', (ot + y - st)=' + (ot + y - st));
  this.ref_line.hline.style.top = (ot + y - st) + 'px';
  this.ref_line.current_y = y;
}

_imcomp_controller.prototype.init_ref_line = function() {
  this.ref_line.hline = document.getElementById('horizontal_line');
  this.hide_element(this.ref_line.hline);
  this.ref_line.current_y = -1;

  window.addEventListener('scroll', function(e) {
    if( this.ref_line.hline.classList.contains('display-inline-block') ) {
      // update the ref_line
      this.ref_line_position(this.ref_line.current_y);
    }
  }.bind(this), false);

  var hline = document.getElementById('horizontal_line');
  hline.addEventListener('mouseup', function(e) {
    this.hide_element(this.ref_line.hline);
    this.ref_line.current_y = -1;
  }.bind(this), false);
}


///
/// zoom
///
_imcomp_controller.prototype.zoom_event_handler = function(e) {
  var id = e.target.id;
  // var type = this.get_container_type(id);
  for ( var type in this.type_list ) {
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
}

_imcomp_controller.prototype.zoom_update_level = function() {
  for( var type in this.type_list ) {
    var switch_name = this.type_list[type] + '_zoom';
    var e = document.getElementById(switch_name);

    if( this.v.now[type].sid_suffix.endsWith('_via') ) {
      this.v.now[type].zoom.is_enabled = false;
      this.m.via[type].c.zoom_deactivate();
      this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
      this.m.via[type].c.zoom_activate();
      this.v.now[type].zoom.is_enabled = true;
    } else {
      this.v.now[type].zoom.is_enabled = false;
      this.disable_image_zoom(type);
      this.enable_image_zoom(type);
      this.v.now[type].zoom.is_enabled = true;
    }
  }
}


_imcomp_controller.prototype.disable_image_zoom = function(type) {
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

_imcomp_controller.prototype.enable_image_zoom = function(type) {
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

  show_message('To <span class="blue">freeze the zoom region</span>, click the left mouse button when zoom is active. Click the left mouse button again to unfreeze the zoom region.');
}

_imcomp_controller.prototype.image_zoom_mousedown_handler = function(e) {
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

_imcomp_controller.prototype.image_zoom_mouseout_handler = function(e) {
  var type = this.get_container_type(e.currentTarget.id);
  if( ! this.v.now[type].zoom.is_frozen ) {
    var content_prefix = e.currentTarget.id.replace('_top_panel', '');
    var zoom = document.getElementById( content_prefix + '_zoom_panel' );
    zoom.setAttribute('style', 'width:0; height:0;');
  }
}

_imcomp_controller.prototype.image_zoom_mousemove_handler = function(e) {
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
/// Results Manipulation.
/// Deal with results such as saving, exporting as json (in future), etc
///
_imcomp_controller.prototype.hor_slider_mousedown_handler = function(e) {
  this.results.is_hor_slider_pressed = true;
}

_imcomp_controller.prototype.vert_slider_mousedown_handler = function(e) {
  this.results.is_vert_slider_pressed = true;
}

_imcomp_controller.prototype.slider_mousemove_handler = function(e) {
  if ( this.results.is_hor_slider_pressed ) {
    var overlay_div = document.getElementById('hor_slide_overlay');
    var overlay_div_position = overlay_div.getBoundingClientRect();
    // cursor position relative to the overlay div
    var cursor_position = e.pageX - overlay_div_position.left;
    // consider any page scrolling
    cursor_position = cursor_position - window.pageXOffset;

    // checks
    var base_width = document.getElementById('left_content_img_panel').offsetWidth;
    if (cursor_position < 0) cursor_position = 0;
    if (cursor_position > base_width) cursor_position = base_width;

    // slide overlay div
    var slider = document.getElementById('horizontal_slider');
    overlay_div.style.width = cursor_position + "px";
    slider.style.left = overlay_div.offsetWidth - (slider.offsetWidth / 2) + "px";
  }

  if ( this.results.is_vert_slider_pressed ) {
    var overlay_div = document.getElementById('vert_slide_overlay');
    var overlay_div_position = overlay_div.getBoundingClientRect();
    // cursor position relative to the overlay div
    var cursor_position = e.pageY - overlay_div_position.top;
    // consider any page scrolling
    cursor_position = cursor_position - window.pageYOffset;

    // checks
    var base_height = document.getElementById('left_content_img_panel').offsetHeight;
    if (cursor_position < 0) cursor_position = 0;
    if (cursor_position > base_height) cursor_position = base_height;

    // slide overlay div
    var slider = document.getElementById('vertical_slider');
    overlay_div.style.height = cursor_position + "px";
    slider.style.top = overlay_div.offsetHeight - (slider.offsetHeight / 2) + "px";
  }
}

_imcomp_controller.prototype.slider_mouseup_handler = function() {
  if ( this.results.is_hor_slider_pressed ) {
    this.results.is_hor_slider_pressed = false;
  }
  if ( this.results.is_vert_slider_pressed ) {
    this.results.is_vert_slider_pressed = false;
  }
}

_imcomp_controller.prototype.slider_switch_radio_handler = function(e) {
  console.log('radio value: ' + e.target.value);
  if ( e.target.value === 'horizontal' ) {
    this.remove_slider_elem('vertical');
    this.remove_overlay_elem();
    this.add_hor_slider_overlay();
  }
  if ( e.target.value === 'vertical' ) {
    this.remove_slider_elem('horizontal');
    this.remove_overlay_elem();
    this.add_vert_slider_overlay();
  }
}

_imcomp_controller.prototype.update_base_comp_fader = function(e) {
  var slider_val = e.target.value;
  var bubble = document.getElementById('base_comp_fader_bubble');
  if (slider_val > 0 && slider_val < 100) {
    bubble.style.left = Math.round(e.clientX - (bubble.offsetWidth / 2) - 12) + 'px';
  }
  bubble.innerHTML = slider_val;
  var overlay_div = document.getElementById('hor_slide_overlay');
  var img = overlay_div.getElementsByTagName('img')[0];
  img.style.opacity = (0.01 * slider_val);
}

_imcomp_controller.prototype.base_comp_fader_show_bubble = function(e) {
  var bubble = document.getElementById('base_comp_fader_bubble');
  bubble.style.left = Math.round(e.clientX - (bubble.offsetWidth / 2) - 12) + 'px';
  bubble.style.opacity = '1';
}

_imcomp_controller.prototype.base_comp_fader_hide_bubble = function(e) {
  var bubble = document.getElementById('base_comp_fader_bubble');
  bubble.style.opacity = '0';
}

_imcomp_controller.prototype.base_comp_fader_move_bubble = function(e) {
  var slider_val = document.getElementById('base_comp_fader').value;
  var bubble = document.getElementById('base_comp_fader_bubble');
  console.log('slider value is: ', slider_val);
  if (slider_val > 1 && slider_val < 100) {
     bubble.style.left = Math.round(e.clientX - (bubble.offsetWidth / 2) - 12) + 'px';
  }
}

_imcomp_controller.prototype.hover_right_left = function(e) {
  if ( this.results.active_tab !== 'hover' ) { return; }

  // get the mouse coordinates wrt the image
  var rect = e.target.getBoundingClientRect();
  var x = Math.round(e.clientX - rect.left);

  var left_img_div = document.getElementById('left_content_image');
  var base_width = document.getElementById('left_content_img_panel').offsetWidth;
  var base_width = left_img_div.offsetWidth;
  // if we are more than half the image distance, we switch to left image
  if ( x <= (base_width / 2) ) {
    left_img_div.src = this.get_content_url('base', 'base_crop');
  } else {
    left_img_div.src = this.get_content_url('comp', 'comp_crop_tx');
  }
}


///
/// Toolbar on top handlers.
/// All the logic for icons in the toolsbar
///
_imcomp_controller.prototype.toolbar_save_handler = function(e) {
  if ( this.v.now['base'].sid_suffix.endsWith('_via') ) {
    show_message('Please compare first to save visualisation of results.');
    return;
  }
  var im1 = document.getElementById( this.type_list['base'] + '_image' );
  var fn1 = this.m.files[0].name; // base file name
  var w1 = im1.naturalWidth;
  var h1 = im1.naturalHeight;

  var vpad = 60;
  var offset = 10;

  var canvas_width = 3*offset + w1;
  var canvas_height = vpad + h1;

  console.log(im1);
  // console.log(im2);
  console.log('canvas = ' + canvas_width + 'x' + canvas_height);

  var c = document.createElement('canvas');
  c.width = canvas_width;
  c.height = canvas_height;
  var ctx = c.getContext('2d', {alpha:false});
  ctx.font = '10px Sans';
  ctx.fillStyle = 'black';
  ctx.fillRect(0, 0, canvas_width, canvas_height);

  ctx.fillStyle = 'yellow';
  var char_width  = ctx.measureText('M').width;
  var char_height = 1.8 * char_width;

  var ts = new Date().toString();
  ctx.fillText('Base: ' + fn1, offset, offset + char_height);
  // ctx.fillText('Comp: ' + fn2, 2*offset + w1, offset + char_height);
  ctx.fillText('Saved using ' +
               _imcomp_about.name + ' ' +
               _imcomp_about.version + ' on ' + ts , offset, canvas_height - offset);

  // draw images
  ctx.drawImage(im1, 0, 0, w1, h1, offset, 2*offset + char_height, w1, h1);
  // ctx.drawImage(im2, 0, 0, w2, h2, 2*offset + w1, 2*offset + char_height, w2, h2);

  // extract image data from canvas
  var img_mime = 'image/jpeg';
  var visualisation_image = c.toDataURL(img_mime);

  //visualisation_image.replace(img_mime, 'image/octet-stream'); // to force download
  // simulate user click to trigger download of image
  var a      = document.createElement('a');
  a.href     = visualisation_image;
  a.download = 'BASE_' + fn1 + '.jpg';

  // simulate a mouse click event
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true
  });

  a.dispatchEvent(event);
}

_imcomp_controller.prototype.toolbar_magnify_handler = function(e) {
  // toggle zoom enable/disable
  if ( this.toolbar.zoom_enabled ) {
    this.toolbar.zoom_enabled = false;
    e.currentTarget.children[0].style.fill = 'black';
  } else {
    this.toolbar.zoom_enabled = true;
    e.currentTarget.children[0].style.fill = 'red';
  }

  for ( var type in this.type_list ) {
    if( this.v.now[type].sid_suffix.endsWith('_via') ) {
      if( this.toolbar.zoom_enabled ) {
        this.v.now[type].zoom.is_enabled = true;
        this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
        this.m.via[type].c.zoom_activate();
      } else {
        this.v.now[type].zoom.is_enabled = false;
        this.m.via[type].c.zoom_deactivate();
      }
    } else {
      if( this.toolbar.zoom_enabled ) {
        // enable image zoom
        this.enable_image_zoom(type);
      } else {
        this.disable_image_zoom(type);
      }
    }
  }
}

_imcomp_controller.prototype.toolbar_back_handler = function(e) {
  if ( this.toolbar.current_page === "home" ) {return;}
  if ( this.toolbar.current_page === "compare" ) {
    _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP1, true);
    this.toolbar.current_page = "home";
  }
  if ( this.toolbar.current_page === "result" ) {
    _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP3, true);
    this.set_now('base', 0);
    this.set_now('comp', 1);
    this.toolbar.current_page = "compare";
    // remove all divs that could have been added in results page
    this.remove_overlay_elem();
    this.remove_slider_elem('all');
    document.getElementById('left_content_container').style.overflow = '';
  }
}

_imcomp_controller.prototype.toolbar_forward_handler = function(e) {
  if ( this.toolbar.current_page === "result" ) {return;}
  if ( this.toolbar.current_page === "compare" ) {
    this.compare_base_comp();
    this.toolbar.current_page = "result";
  }
}

_imcomp_controller.prototype.toolbar_zoomin_handler = function(e) {
  var base_img = document.getElementById('left_content_image');
  var overlay_img = document.getElementById('hor_slide_overlay_img');
  if ( this.results.active_tab === 'fade' || this.results.active_tab === 'slide' ||
       this.results.active_tab === 'overlay') {
    base_img.style.width = this.results.canvas_width;
    base_img.style.height = this.results.canvas_height;
    show_message('Cannot zoom in fade, slide and overlay mode!');
    return;
  }
  base_img.style.width = Math.round(base_img.offsetWidth * 1.1) + 'px';
  base_img.style.height = Math.round(base_img.offsetHeight * 1.1) + 'px';
  // overlay_img.style.width = Math.round(overlay_img.offsetWidth * 1.1) + 'px';
  // overlay_img.style.height = Math.round(overlay_img.offsetHeight * 1.1) + 'px';
}

_imcomp_controller.prototype.toolbar_zoomout_handler = function(e) {
  var base_img = document.getElementById('left_content_image');
  var overlay_img = document.getElementById('hor_slide_overlay_img');
  if ( this.results.active_tab === 'fade' || this.results.active_tab === 'slide' ||
       this.results.active_tab === 'overlay') {
    base_img.style.width = this.results.canvas_width;
    base_img.style.height = this.results.canvas_height;
    show_message('Cannot zoom in fade, slide and overlay mode!');
    return;
  }
  base_img.style.width = Math.round(base_img.offsetWidth * 0.9) + 'px';
  base_img.style.height = Math.round(base_img.offsetHeight * 0.9) + 'px';
  // overlay_img.style.width = Math.round(overlay_img.offsetWidth * 0.9) + 'px';
  // overlay_img.style.height = Math.round(overlay_img.offsetHeight * 0.9) + 'px';

}

_imcomp_controller.prototype.algorithm_change_handler = function(e) {
  switch (e.target.value) {
    case 'identity':
    case 'translation':
    case 'affine':
      this.compare.algorithm = 'ransac_dlt';
      break;
    case 'tps':
      console.log('setting algname to tps');
      this.compare.algorithm = 'robust_ransac_tps';
      break;
    case 'perspective':
    default: // affine
      this.compare.algorithm = 'ransac_dlt';
  }
}

_imcomp_controller.prototype.instruction_step1_handler = function(e) {
  _imcomp_set_panel('step1', true);
  this.toolbar.current_page = 'home';
}

_imcomp_controller.prototype.instruction_step2_handler = function(e) {
  if (this.toolbar.current_page === "compare") {return;}
  // we have less than 2 files uploaded. no point showing compare page.
  if (this.m.files.length <= 1) {return;}

  _imcomp_set_panel(IMCOMP_PANEL_NAME.STEP3, true);
  this.set_now('base', 0);
  this.set_now('comp', 1);
  this.toolbar.current_page = "compare";
  // remove all divs that could have been added in results page
  this.remove_overlay_elem();
  this.remove_slider_elem('all');
  // delete any regions pre-selected
  this.m.via['base'].c.region_select_all();
  this.m.via['base'].c.delete_selected_regions();

}

_imcomp_controller.prototype.instruction_step3_handler = function(e) {
  if (this.toolbar.current_page === "result" ||
      this.toolbar.current_page === "home") {return;}

  this.compare_base_comp();
  this.toolbar.current_page = "result";
}


_imcomp_controller.prototype.show_demo = function(e) {
  // parent of the img element is the div wrapping it
  if (e.target.parentNode.id === "sample_image_set_one_base" ||
      e.target.parentNode.id === "sample_image_set_one_comp") {
    this.m.demo_files = ['demo/painting_base.png', 'demo/painting_comp.png'];
  }
  if (e.target.parentNode.id === "sample_image_set_two_base" ||
      e.target.parentNode.id === "sample_image_set_two_comp") {
    this.m.demo_files = ['demo/portrait_base.png', 'demo/portrait_comp.png'];
  }
  if (e.target.parentNode.id === "sample_image_set_three_base" ||
  e.target.parentNode.id === "sample_image_set_three_comp") {
    this.m.demo_files = ['demo/cartoon_base.png', 'demo/cartoon_comp.png'];
  }
  if (e.target.parentNode.id === "sample_image_set_four_base" ||
      e.target.parentNode.id === "sample_image_set_four_comp") {
    this.m.demo_files = ['demo/book1_base.jpg', 'demo/book1_comp.jpg'];
  }
  if (e.target.parentNode.id === "sample_image_set_five_base" ||
      e.target.parentNode.id === "sample_image_set_five_comp" ) {
    this.m.demo_files = ['demo/book2_base.jpg', 'demo/book2_comp.jpg'];
  }
  if (e.target.parentNode.id === "sample_image_set_six_base" ||
      e.target.parentNode.id === "sample_image_set_six_comp") {
    this.m.demo_files = ['demo/book3_base.jpg', 'demo/book3_comp.jpg'];
  }
  console.log('set demo files to: ', this.m.demo_files);
  this.m.fetch_show_demo_pair(e);
}
