function _traherne_controller() {
  this.type_list = {'base':'left_content', 'comp':'right_content'};
  this.type_description = {'base':'Base', 'comp':'Comp.'};

  this.state = {};
  this.now = {};
  this.now.base = {};
  this.now.comp = {};

  this.config = {};
  this.config.upload = {};
  this.config.upload.MAX_IMG_DIM_PX = 1200;
  this.config.imcomp_server_upload_uri = "/imcomp/_upload";
  this.config.imcomp_server_compare_uri = "/imcomp/_compare";

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
}

_traherne_controller.prototype.update_files = function(type, e) {
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
  var filelist = this.m.get_filelist(type);
  this.update_view_filelist(type, filelist);
  this.set_now(type, 0); // show first image

  var sid_suffix = type + '_via';

  this.content_selector_set_state(type, sid_suffix, true);
  this.set_content(type, sid_suffix);

  this.enable_switch(type, '_zoom');
}

_traherne_controller.prototype.update_view_filelist = function(type, filelist) {
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
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

  this.on_now_update(type);
}

_traherne_controller.prototype.on_now_update = function(type) {
  // update the selected item in filelist dropdown
  var list_name = type + '_img_filename_list';
  var list = document.getElementById(list_name);
  var now_findex = this.v.now[type].findex;

  if( list.options.length ) {
    for ( var i=0; i<list.options.length; i++ ) {
      if ( list.options[i].value == now_findex ) {
        list.options[i].setAttribute('selected', 'selected');
      } else {
        if ( list.options[i].hasAttribute('selected') ) {
          list.options[i].removeAttribute('selected')
        }
      }
    }
  }

  this.update_now_file_status(type);
}

_traherne_controller.prototype.update_now = function(type, e) {
  var findex = e.target.value;
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
      this.v.msg('Add ' + type + ' images by clicking "Load ' + type_name + ' Images" button');
      return;
    }
  }

  if( !this.is_base_region_selected() ) {
    this.v.msg('To compare, you must define a region of interest in the base image. Keeping the right mouse button pressed on the base image, click and drag to draw a region of interest.');
    return;
  }

  var findex1 = this.v.now['base'].findex;
  var findex2 = this.v.now['comp'].findex;
  var c = new _traherne_compare_instance(findex1, findex2);
  this.m.upload['base'][findex1].then( function(upload_id1) {
    c.upload_id1 = upload_id1;
    c.scale1 = this.m.upload_scale['base'][c.findex1];
    var fid1 = this.m.index_to_fid['base'][c.findex1];
    var rid = this.m.via['base'].v.now.all_rid_list[0];
    c.region1 = this.m.via['base'].m.regions[fid1][rid].dimg.slice(0);

    this.m.upload['comp'][findex2].then( function(upload_id2) {
      c.upload_id2 = upload_id2;
      c.scale2 = this.m.upload_scale['comp'][c.findex2];
      this.compare.is_ongoing = true;
      this.compare.promise = this.m.compare_img_pair(c);
    }.bind(this));
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
}

_traherne_controller.prototype.on_compare_end = function() {
  this.compare.end_time = new Date();
  this.compare.is_ongoing = false;
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
  this.v.msg('Compare status: ' + this.m.compare_status.msg);
}

_traherne_controller.prototype.on_compare_success = function() {
  // note: this.compare.result contains the result
  var time = this.compare.end_time - this.compare.start_time;
  var msg = 'Comparison completed in ' + Math.round(time/1000) + ' sec.';
  this.v.msg(msg);

  this.show_compare_result();

  this.enable_all_content_selectors();
}

_traherne_controller.prototype.on_compare_failure = function() {
  var time = this.compare.end_time - this.compare.start_time;
  var msg = 'Comparison failed';
  this.v.msg(msg);
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
}

_traherne_controller.prototype.set_content = function(type, sid_suffix) {
  var via = document.getElementById( this.type_list[type] + '_via_panel' );
  var img = document.getElementById( this.type_list[type] + '_image' );

  if( sid_suffix.endsWith('_via') ) {
    this.hide_element(img);
    this.show_element(via);
    this.disable_all_switches('_toggle'); // toggle requires image content
    this.enable_switch(type, '_zoom');
  } else {
    this.hide_element(via);
    this.show_element(img);
    this.clear_toggle( this.type_list[type] ); // clear any existing toggle
    this.enable_all_switches('_toggle');
    this.disable_switch(type, '_zoom'); // @todo: enable zoom for all images

    var content_url = this.get_content_url(type, sid_suffix);
    img.setAttribute('src', content_url);
  }

  this.content_selector_set_checked(type, sid_suffix);
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

_traherne_controller.prototype.ref_line_clear = function() {
  this.ref_line.ctx.clearRect(0, 0, this.ref_line.c.width, this.ref_line.c.height);
}

_traherne_controller.prototype.ref_line_marker_draw = function(y) {
  this.ref_line.ctx.fillRect(0,
                             y - this.ref_line.LINE_HEIGHT,
                             this.ref_line.c.width,
                             2*this.ref_line.LINE_HEIGHT);
  this.ref_line.current_y = y;
}

_traherne_controller.prototype.ref_line_position = function(y) {
  var st = document.documentElement.scrollTop;
  var ot = document.getElementById('ref_line_container').offsetTop;
  this.ref_line.hline.style.top = (ot + y - st) + 'px';
}

_traherne_controller.prototype.ref_line_resize_canvas = function() {
  // update the dim. of canvas to match the content
  var parent = document.getElementById('ref_line_container');
  this.ref_line.c.width = Math.floor(parent.clientWidth);
  this.ref_line.c.height = Math.floor(parent.clientHeight) - 4;
}

_traherne_controller.prototype.init_ref_line = function() {
  this.ref_line.hline = document.getElementById('horizontal_line');
  this.hide_element(this.ref_line.hline);

  this.ref_line.c = document.getElementById('ref_line_canvas');
  this.ref_line.ctx = this.ref_line.c.getContext('2d');
  this.ref_line.current_y = 0;
  this.ref_line.LINE_HEIGHT = 4;
  this.ref_line_resize_canvas();
  /*
  this.ref_line.c.addEventListener('mousedown', function(e) {
    var c = document.getElementById('ref_line_container');
    var cot = c.offsetTop;
    var cst = c.scrollTop;
    var cct = c.clientTop;
    var pst = document.documentElement.scrollTop;
    var ltop = this.ref_line.hline.style.top;

    console.log('ref_line container: clientTop='+cct+', offsetTop='+cot+', scrollTop='+cst+', page.scrollTop='+pst+', line.top='+ltop);
  }.bind(this));
  */

  window.addEventListener('scroll', function(e) {
    if( this.ref_line.hline.classList.contains('display-inline-block') ) {
      // update the ref_line
      this.ref_line_position(this.ref_line.current_y);
    }
  }.bind(this));

  this.ref_line.c.addEventListener('mouseup', function(e) {
    this.ref_line_resize_canvas();

    var y = e.offsetY;
    this.ref_line_clear();

    if( Math.abs(y - this.ref_line.current_y) <= this.ref_line.LINE_HEIGHT ) {
      this.hide_element(this.ref_line.hline);
      return;
    }

    this.ref_line_marker_draw(y);
    this.ref_line_position(y);
    this.show_element(this.ref_line.hline);
  }.bind(this));
}


///
/// zoom
///
_traherne_controller.prototype.zoom_event_handler = function(e) {
  var id = e.target.id;
  var type = this.get_container_type(id);
  if( e.target.checked ) {
    this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
    this.m.via[type].c.zoom_activate();
  } else {
    this.m.via[type].c.zoom_deactivate();
  }
}
_traherne_controller.prototype.zoom_update_level = function() {
  for( var type in this.type_list ) {
    var switch_name = this.type_list[type] + '_zoom';
    var e = document.getElementById(switch_name);
    if( e.checked ) {
      this.m.via[type].c.zoom_deactivate();
      this.m.via[type].v.zoom.scale = this.v.theme.ZOOM_LEVEL;
      this.m.via[type].c.zoom_activate();
    }
  }
}
