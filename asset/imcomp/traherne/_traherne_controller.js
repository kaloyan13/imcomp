function _traherne_controller() {
  this.type_list = {'base':'Base', 'comp':'Comp.'};

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
}

function _traherne_compare_instance(findex1, findex2) {
  this.findex1 = findex1;
  this.findex2 = findex2;
}

_traherne_controller.prototype.init = function( traherne_model, traherne_view ) {
  this.m = traherne_model;
  this.v = traherne_view;
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
    status_symbol = '&#9745;';
    break;
  case 'ERR':
    status_symbol = '&#9746;';
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
    console.log(type)
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
  console.log('_traherne_controller.prototype.on_compare_end()');
  this.compare.promise.then( function(c) {
    console.log(c);
    if( c.result.status === 'OK' ) {
      var time = this.compare.end_time - this.compare.start_time;
      var msg = 'Comparison completed in ' + (time/1000) + ' sec.';
      this.v.msg(msg);
      this.show_compare_result(c);
    }
  }.bind(this));
}

_traherne_controller.prototype.on_compare_status_update = function() {
  this.v.msg('Compare status: ' + this.m.compare_status.msg);
}

_traherne_controller.prototype.show_compare_result = function(c) {
  // in base panel, show file1 crop
  this.set_panel_content('base', c.result.file1_crop);

  // in comp panel, show file2 transformed + crop
  this.set_panel_content('comp', c.result.file2_crop_tx);
}

_traherne_controller.prototype.set_panel_content = function(type, url) {
  var via = document.getElementById( type + '_via_panel' );
  var img = document.getElementById( type + '_image' );

  if( url == 'via_panel' ) {
    this.hide_element(img);
    this.show_element(via);
  } else {
    this.hide_element(via);
    this.show_element(img);
    img.setAttribute('src', url);
  }
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
