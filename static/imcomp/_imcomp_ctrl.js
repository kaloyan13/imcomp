function _imcomp_ctrl() {
  this.image1_via = new _via();
  this.image2_via = new _via();

  this.toolbox = {};
  this.toolbox.toggle = {};
  this.toolbox.toggle.DEFAULT_INTERVAL_MS = 500;
  this.toolbox.toggle.DEFAULT_INTERVAL_DECREMENT = 100;

  this.compare_img_mouse_event_handler = this.compare_img_mouse_event_listener.bind(this);

  this.file_upload_status = {};
}

///
/// initialization routines
///
_imcomp_ctrl.prototype.init = function(imcomp_model, imcomp_view) {
  //console.log("Initializing _imcomp_ctrl ...");
  this.m = imcomp_model;
  this.v = imcomp_view;

  // add user input handlers
  window.addEventListener("keydown", this.keyboard_input_handler.bind(this));

  this.show_select_files_panel();
  //this.show_compare_panel();

  // initialize the user image panels (with VGG Image Annotator)
  // debug
  var image1_display_panel = document.getElementById("user_image1");
  this.image1_via.init(image1_display_panel);

  var image2_display_panel = document.getElementById("user_image2");
  this.image2_via.init(image2_display_panel);

  var image1_filename_list = document.getElementById('image1_filename_list');
  image1_filename_list.addEventListener('change', this.image1_update_file.bind(this), false);
  var image2_filename_list = document.getElementById('image2_filename_list');
  image2_filename_list.addEventListener('change', this.image2_update_file.bind(this), false);

  this.init_via_hooks();

  // @todo: experiment: shared files
  // this will avoid extra memory usage
  //this.image2_via.m.files = this.image1_via.m.files;

  // debug
  //this.update_compare_result();
  //this.difference_image('http://localhost:9967/dev_test/fceaeb_crop.jpg', 'http://localhost:9967/dev_test/f84958_crop_tx.jpg')
}

// ensures that only one region is present at any given time
_imcomp_ctrl.prototype.init_via_hooks = function() {
  this.image1_via.c.add_hook(this.image1_via.c.hook.id.REGION_ADDED, function(param) {
    //console.log('hook: region added : fileid=' + param.fileid + ', rid=' + param.rid);

    // delete old region from image1
    if ( this.image1_via.v.now.all_rid_list.length > 1 ) {
      image1_old_region = [ this.image1_via.v.now.all_rid_list[0] ];
      this.image1_via.c.region_delete(image1_old_region);
    }

    // delete all regions from image2
    if ( this.image2_via.v.now.all_rid_list.length ) {
      var image2_regions = this.image2_via.v.now.all_rid_list.slice(0);
      this.image2_via.c.region_delete( image2_regions );
    }
  }.bind(this));

  this.image2_via.c.add_hook(this.image2_via.c.hook.id.REGION_ADDED, function(param) {
    //console.log('hook: region added : fileid=' + param.fileid + ', rid=' + param.rid);

    // delete old region from image2
    if ( this.image2_via.v.now.all_rid_list.length > 1 ) {
      image2_old_region = [ this.image2_via.v.now.all_rid_list[0] ];
      this.image2_via.c.region_delete(image2_old_region);
    }

    // delete all regions from image1
    if ( this.image1_via.v.now.all_rid_list.length ) {
      var image1_regions = this.image1_via.v.now.all_rid_list.slice(0);
      this.image1_via.c.region_delete( image1_regions );
    }
  }.bind(this));
}

_imcomp_ctrl.prototype.show_select_files_panel = function() {
  if ( this.v.compare_panel.classList.contains('display-table') ) {
    this.v.compare_panel.classList.remove('display-table');
  }
  this.v.compare_panel.classList.add('display-none');

  if ( this.v.select_files_panel.classList.contains('display-none') ) {
    this.v.select_files_panel.classList.remove('display-none');
  }
  this.v.select_files_panel.classList.add('display-table');
}

_imcomp_ctrl.prototype.show_compare_panel = function() {
  if ( this.v.compare_panel.classList.contains('display-none') ) {
    this.v.compare_panel.classList.remove('display-none');
  }
  this.v.compare_panel.classList.add('display-table');

  if ( this.v.select_files_panel.classList.contains('display-table') ) {
    this.v.select_files_panel.classList.remove('display-table');
  }
  this.v.select_files_panel.classList.add('display-none');

  document.getElementById('top_message_panel').innerHTML = 'Draw a region and click Compare Images';
}

///
/// keyboard input handlers
///
_imcomp_ctrl.prototype.keyboard_input_handler = function(e) {
  e.stopPropagation();
}


///
/// add/remove files
///
_imcomp_ctrl.prototype.select_local_files = function() {
  // ref: https://developer.mozilla.org/en-US/docs/Using_files_from_web_applications
  this.v.local_file_selector.click();
}

_imcomp_ctrl.prototype.add_user_sel_local_files = function(e) {
  e.stopPropagation();
  this.show_compare_panel();
  this.v.disable_all_toolbox_buttons();

  var user_selected_files = e.target.files;
  var n = user_selected_files.length;
  /*
  if ( n >= 2 ) {
    this.show_compare_panel();
  } else {
    window.alert('In order to compare, you must choose at least 2 images!');
    return;
  }
  */
  var promises1 = [];
  var promises2 = [];
  for (var i=0; i < n; i++) {
    //console.log('Adding local file : ' + user_selected_files[i].name);
    //var findex = this.m.add_file_local(user_selected_files[i]);

    promises1.push( this.image1_via.m.add_file_local(user_selected_files[i]) );
    promises2.push( this.image2_via.m.add_file_local(user_selected_files[i]) );
  }

  Promise.all(promises1).then( function(result) {
    this.image1_via.c.load_file_from_index( 0 );

    // update the dropdown list of image filenames
    var n = result.length;
    for( var i=0; i<n; i++ ) {
      var fid = result[i];
      if ( !fid.startsWith("Error") ) {
        var option1  = document.createElement('option');
        if ( i == 0 ) {
          option1.setAttribute('selected', 'selected');
        }
        if ( i == 1 ) {
          option1.setAttribute('disabled', 'disabled');
        }
        var findex   = this.image1_via.m.files.file_index_list[fid];
        var filename = this.image1_via.m.files.metadata[fid].filename;

        option1.value = fid;
        option1.text  = '[' + findex + '] ' + filename;
        image1_filename_list.add(option1, null);

        // upload file to imcomp server
        var file_source  = 'local';
        var file_content = this.image1_via.m.files.content[fid];
        this.m.upload_file(fid, file_content, file_source);
      }
    }
  }.bind(this));

  Promise.all(promises2).then( function(result) {
    this.image2_via.c.load_file_from_index( 1 );

    // update the dropdown list of image filenames
    var n = result.length;
    for( var i=0; i<n; i++ ) {
      var fid = result[i];
      if ( !fid.startsWith("Error") ) {
        var option2  = document.createElement('option');
        if ( i == 0 ) {
          option2.setAttribute('disabled', 'disabled');
        }
        if ( i == 1 ) {
          option2.setAttribute('selected', 'selected');
        }

        var findex   = this.image2_via.m.files.file_index_list[fid];
        var filename = this.image2_via.m.files.metadata[fid].filename;

        option2.value = fid;
        option2.text  = '[' + findex + '] ' + filename;
        image2_filename_list.add(option2, null);
      }
    }
  }.bind(this));
}

_imcomp_ctrl.prototype.update_file_upload_progress = function(local_fid, message) {
  this.file_upload_status[local_fid] = message;

  var file1_selector = document.getElementById('image1_filename_list');
  if( file1_selector.selectedIndex >= 0 ) {
    var file1_local_fid = file1_selector.options[file1_selector.selectedIndex].value;
    if ( file1_local_fid === local_fid ) {
      document.getElementById('file1_message_panel').innerHTML = message;
    }
  }

  var file2_selector = document.getElementById('image2_filename_list');
  if( file2_selector.selectedIndex >= 0 ) {
    var file2_local_fid = file2_selector.options[file2_selector.selectedIndex].value;
    if ( file2_local_fid === local_fid ) {
      document.getElementById('file2_message_panel').innerHTML = message;
    }
  }
}

_imcomp_ctrl.prototype.image1_update_file = function(e) {
  e.stopPropagation();
  var local_fid = e.target.value;
  this.image1_via.c.load_file( local_fid );
  document.getElementById('file1_message_panel').innerHTML = this.file_upload_status[local_fid];

  // file selected as image1 should not be selectable in image2
  var file2_selector = document.getElementById('image2_filename_list');
  if( file2_selector.options.length ) {
    for ( var i=0; i<file2_selector.options.length; i++ ) {
      if ( file2_selector.options[i].value == local_fid ) {
        file2_selector.options[i].setAttribute('disabled', 'disabled');
      } else {
        if ( file2_selector.options[i].hasAttribute('disabled') ) {
          file2_selector.options[i].removeAttribute('disabled')
        }
      }
    }
  }
}

_imcomp_ctrl.prototype.image2_update_file = function(e) {
  e.stopPropagation();
  var local_fid = e.target.value;
  this.image2_via.c.load_file( local_fid );
  document.getElementById('file2_message_panel').innerHTML = this.file_upload_status[local_fid];

  // file selected as image2 should not be selectable in image1
  var file1_selector = document.getElementById('image1_filename_list');
  if( file1_selector.options.length ) {
    for ( var i=0; i<file1_selector.options.length; i++ ) {
      if ( file1_selector.options[i].value == local_fid ) {
        file1_selector.options[i].setAttribute('disabled', 'disabled');
      } else {
        if ( file1_selector.options[i].hasAttribute('disabled') ) {
          file1_selector.options[i].removeAttribute('disabled')
        }
      }
    }
  }
}

_imcomp_ctrl.prototype.compare = function() {
  // remove existing compare image (if any)
  var img = document.getElementById('compare_result_image');
  img.setAttribute('src', '');

  this.compare_ongoing();

  // stop toggle
  this.clear_toggle_timer();
  this.v.toolbox_view_clear_all_pressed();

  var file1_local_fid = this.image1_via.v.now.fileid;
  var file2_local_fid = this.image2_via.v.now.fileid;
  var url = this.m.server.imcompserver_url + '_compare';

  document.getElementById('top_message_panel').innerHTML = '';
  document.getElementById('bottom_message_panel').innerHTML = '';
  document.getElementById('file_info_panel').innerHTML = '';

  this.m.session.remote_files[file1_local_fid].then( function(file1_remote_fid) {
    this.m.session.remote_files[file2_local_fid].then( function(file2_remote_fid) {
      var image1_rid_list = [];
      var image2_rid_list = [];

      for( var r in this.image1_via.m.regions[file1_local_fid] ) {
        if( this.image1_via.m.regions[file1_local_fid].hasOwnProperty(r) ) {
          image1_rid_list.push(r);
        }
      }
      for( var r in this.image2_via.m.regions[file2_local_fid] ) {
        if( this.image2_via.m.regions[file2_local_fid].hasOwnProperty(r) ) {
          image2_rid_list.push(r);
        }
      }

      var args = [];
      var file1_scale = this.m.session.remote_files_scale[file1_local_fid];
      var file2_scale = this.m.session.remote_files_scale[file2_local_fid];
      if ( image1_rid_list.length === 1 ) {
        args.push('file1=' + file1_remote_fid);
        args.push('file2=' + file2_remote_fid);
        var rid = image1_rid_list[0];
        var dimg = this.image1_via.m.regions[file1_local_fid][rid].dimg.slice(0);
        if ( file1_scale != 1.0 ) {
          for ( var i=0; i<dimg.length; i++ ) {
            dimg[i] = Math.round(dimg[i] * file1_scale);
          }
        }
        args.push('region=' + dimg.join(','));
      } else {
        if ( image2_rid_list.length === 1 ) {
          args.push('file1=' + file2_remote_fid);
          args.push('file2=' + file1_remote_fid);
          var rid = image2_rid_list[0];
          var dimg = this.image2_via.m.regions[file2_local_fid][rid].dimg.slice(0);
          if ( file2_scale != 1.0 ) {
            for ( var i=0; i<dimg.length; i++ ) {
              dimg[i] = Math.round(dimg[i] * file2_scale);
            }
          }
          args.push('region=' + dimg.join(','));
        } else {
          this.show_imcomp_server_error('Please draw a region first. To draw a region, click and drag mouse over either of the two images.');
          this.compare_done();
          return;
        }
      }

      var cr = new XMLHttpRequest();
      cr.addEventListener('load', function() {
        var response_str = cr.responseText;
        var response     = JSON.parse(response_str).IMAGE_HOMOGRAPHY[0];

        if( response.status === 'OK' ) {
          response.file1_local_fid = file1_local_fid;
          response.file2_local_fid = file2_local_fid;
          response.file1_remote_fid = file1_remote_fid;
          response.file2_remote_fid = file2_remote_fid;

          this.m.set_compare_result(response);
        } else {
          this.show_imcomp_server_error(response.status_message);
        }
        this.compare_done();
      }.bind(this));

      cr.addEventListener('timeout', function() {
        this.show_imcomp_server_error('imcomp server responsed timed out!');
      }.bind(this));
      cr.addEventListener('error', function() {
        this.show_imcomp_server_error('error sending request to imcomp server!');
      }.bind(this));
      cr.addEventListener('abort', function() {
        this.show_imcomp_server_error('aborted sending request to imcomp server!');
      }.bind(this));

      cr.open('POST', url + '?' + args.join('&'));
      cr.timeout = 40000; // 20 sec
      cr.send();

      //cr.open('POST', url);
      //cr.timeout = 40000; // 20 sec
      //cr.send(args.join('&'));
    }.bind(this));
  }.bind(this));

  this.m.session.remote_files[file1_local_fid].catch( function() {
    this.show_imcomp_server_error('failed to upload file to imcomp server!');
  }.bind(this));
  this.m.session.remote_files[file2_local_fid].catch( function() {
    this.show_imcomp_server_error('failed to upload file to imcomp server!');
  }.bind(this));
}

_imcomp_ctrl.prototype.show_imcomp_server_error = function(msg) {
  var bp = document.getElementById('bottom_message_panel');
  bp.innerHTML = '<span style="color:red;">Error: ' + msg + '</span>';

  var img = document.getElementById('compare_result_image');
  img.setAttribute('src', '');
  document.getElementById('file_info_panel').innerHTML = '';
  this.compare_done();
}

_imcomp_ctrl.prototype.update_compare_result = function() {
  this.show_compare_result();
  //this.show_file2_region();
  this.v.enable_all_toolbox_buttons();
}

_imcomp_ctrl.prototype.show_file2_region = function() {
  this.image2_via.v.now.region_shape = this.image2_via.v.settings.REGION_SHAPE.POLYGON;
  this.image2_via.v.nvertex = [];
  var scale_inv = 1 / this.image2_via.v.now.tform.scale;
  for ( var i=0; i<this.m.now.compare_result.file2_region.length; i++ ) {
    this.image2_via.v.nvertex.push( Math.round(this.m.now.compare_result.file2_region[i] * scale_inv) );
  }

  this.image2_via.c.add_region_from_nvertex();
  this.image2_via.v.set_state( this.image2_via.v.state.IDLE );
}

_imcomp_ctrl.prototype.show_compare_result = function() {
  // stop toggle
  this.clear_toggle_timer();

  this.v.toolbox_view_clear_all_pressed();
  this.clear_compare_img_mouse_event_listeners();

  var img = document.getElementById('compare_result_image');
  img.setAttribute('src', this.m.now.compare_result.file1.crop_url);

  var fid = this.m.now.compare_result.file1.local_fid;
  var filename = this.image1_via.m.files.metadata[fid].filename;
  document.getElementById('file_info_panel').innerHTML = 'File: ' + filename;

  this.set_compare_img_mouse_event_listeners();

  var mp = document.getElementById('top_message_panel');
  mp.innerHTML = 'Click on image to switch between regions being compared.';

  var bp = document.getElementById('bottom_message_panel');
  var h = 'H = [' + this.m.now.compare_result.server_response.homography.join(', ') + ']';
  var inliers = 'RANSAC best inliers count = ' + this.m.now.compare_result.server_response.best_inliers_count;
  bp.innerHTML = h + '<br/>' + inliers;

  document.getElementById('toolbox_view_overlay').classList.add('pressed');
}

_imcomp_ctrl.prototype.compare_img_mouse_event_listener = function(e) {
  if(e.type === 'mousedown') {
    var img = document.getElementById('compare_result_image');
    var fid = this.m.now.compare_result.file2.local_fid;
    var filename = this.image2_via.m.files.metadata[fid].filename;

    img.setAttribute('src', this.m.now.compare_result.file2.crop_tx_url);
    document.getElementById('file_info_panel').innerHTML = 'File: ' + filename;
  }
  if(e.type === 'mouseup') {
    var img = document.getElementById('compare_result_image');
    var fid = this.m.now.compare_result.file1.local_fid;
    var filename = this.image1_via.m.files.metadata[fid].filename;

    img.setAttribute('src', this.m.now.compare_result.file1.crop_url);
    document.getElementById('file_info_panel').innerHTML = 'File: ' + filename;
  }
}

_imcomp_ctrl.prototype.compare_result_toggle = function() {
  document.getElementById('top_message_panel').innerHTML = '';
  this.v.toolbox_view_clear_all_pressed();
  this.clear_compare_img_mouse_event_listeners();

  if ( toggle_timer ) {
    // toggle view already present, decrease timer interval
    this.clear_toggle_timer();
    if ( this.v.now.toggle_interval_ms <= 100 ) {
      this.v.now.toggle_interval_ms = this.toolbox.toggle.DEFAULT_INTERVAL_MS;
    } else {
      this.v.now.toggle_interval_ms = this.v.now.toggle_interval_ms - this.toolbox.toggle.DEFAULT_INTERVAL_DECREMENT;
    }
    this.activate_compare_result_toggle();
  } else {
    // first time
    this.v.now.toggle_interval_ms = this.toolbox.toggle.DEFAULT_INTERVAL_MS;
    this.activate_compare_result_toggle();
  }
}

_imcomp_ctrl.prototype.activate_compare_result_toggle = function() {
  document.getElementById('toolbox_view_toggle').classList.add('pressed');
  var img = document.getElementById('compare_result_image');

  this.set_compare_img_mouse_event_listeners();
  this.toolbox.toggle.next_event = 'mousedown';
  toggle_timer = setInterval( function() {
    this.trigger_mouse_event(img, this.toolbox.toggle.next_event);
    if ( this.toolbox.toggle.next_event === 'mousedown' ) {
      this.toolbox.toggle.next_event = 'mouseup';
    } else {
      this.toolbox.toggle.next_event = 'mousedown';
    }
  }.bind(this), this.v.now.toggle_interval_ms);
  var mp = document.getElementById('top_message_panel');
  mp.innerHTML = 'Toggle interval = ' + this.v.now.toggle_interval_ms + 'ms (click Toggle button again to change toggle speed)';
}

_imcomp_ctrl.prototype.clear_toggle_timer = function() {
  clearInterval(toggle_timer);
  toggle_timer = 0;
}

_imcomp_ctrl.prototype.trigger_mouse_event = function(element, event_type) {
  var e = document.createEvent('MouseEvents');
  e.initEvent(event_type, true, true);
  element.dispatchEvent(e);
}

_imcomp_ctrl.prototype.show_difference_result = function() {
  // stop toggle
  this.clear_toggle_timer();

  this.v.toolbox_view_clear_all_pressed();
  this.clear_compare_img_mouse_event_listeners();

  var img = document.getElementById('compare_result_image');
  img.setAttribute('src', this.m.now.compare_result.file1_file2_diff);

  document.getElementById('toolbox_view_difference').classList.add('pressed');

  var msg = 'Difference image: <span style="color:red">image1&nbsp;&rarr;&nbsp;red channel</span>';
  msg += '&nbsp;&nbsp;<span style="color:green">image2&nbsp;&rarr;&nbsp;green channel</span>'
  document.getElementById('top_message_panel').innerHTML = msg;

  document.getElementById('file_info_panel').innerHTML = '';
/*
  var fid1 = this.m.now.compare_result.file1.local_fid;
  var filename1 = this.image1_via.m.files.metadata[fid1].filename;
  msg += '<span style="color:red">' + filename1 + ' &rarr; red channel</span>&nbsp;,&nbsp;';
  var fid2 = this.m.now.compare_result.file2.local_fid;
  var filename2 = this.image1_via.m.files.metadata[fid2].filename;
  msg += '<span style="color:green">' + filename2 + ' &rarr; green channel</span>';
  mp.innerHTML = msg;
*/
}

/*
_imcomp_ctrl.prototype.compare_result_difference = function() {
  // load images in canvas
  var canvas1 = document.createElement('canvas');
   // create difference image canvas

  // set src
}

_imcomp_ctrl.prototype.difference_image = function(url1, url2) {
  return new Promise( function(ok_callback, err_callback) {
    var im1 = new Image();
    im1.addEventListener('load', function() {
      var im2 = new Image();
      im2.addEventListener('load', function() {
        var canvas1 = document.createElement('canvas');
        //var canvas1 = document.getElementById('compare_difference_image');
        canvas1.width = im1.naturalWidth;
        canvas1.height = im1.naturalHeight;
        var ctx1 = canvas1.getContext('2d');
        ctx1.drawImage(im1, 0, 0);

        var canvas2 = document.createElement('canvas');
        //var canvas1 = document.getElementById('compare_difference_image');
        canvas2.width = im2.naturalWidth;
        canvas2.height = im2.naturalHeight;
        var ctx2 = canvas2.getContext('2d');
        ctx2.drawImage(im2, 0, 0);

        var im1data = ctx1.getImageData(0, 0, im1.naturalWidth, im1.naturalHeight).data;
        var im2data = ctx2.getImageData(0, 0, im2.naturalWidth, im2.naturalHeight).data;

        console.log(im1data.length);
        console.log(im2data.length);
        //var p = document.getElementById('compare_result_image');
        //var dataurl = canvas1.toDataURL('image/jpeg', 0.8);
        //console.log(dataurl);
        //p.setAttribute('src', dataurl);
      });
      im2.src = url2;

      //var p = document.getElementById('compare_result_image');
      //p.setAttribute('src', im1.src);
    });

    im1.src = url1;
 }.bind(this));
}
*/

_imcomp_ctrl.prototype.compare_ongoing = function() {
  var p = document.getElementById('compare');
  if ( !p.classList.contains('button_action_ongoing') ) {
    p.classList.add('button_action_ongoing');
  }
}

_imcomp_ctrl.prototype.compare_done = function() {
  var p = document.getElementById('compare');
  if ( p.classList.contains('button_action_ongoing') ) {
    p.classList.remove('button_action_ongoing');
  }
}

_imcomp_ctrl.prototype.set_compare_img_mouse_event_listeners = function() {
  var img = document.getElementById('compare_result_image');
  img.addEventListener('mousedown', this.compare_img_mouse_event_handler, false);
  img.addEventListener('mouseup'  , this.compare_img_mouse_event_handler, false);
}

_imcomp_ctrl.prototype.clear_compare_img_mouse_event_listeners = function() {
  var img = document.getElementById('compare_result_image');
  img.removeEventListener('mousedown', this.compare_img_mouse_event_handler, false);
  img.removeEventListener('mouseup'  , this.compare_img_mouse_event_handler, false);
}

///
/// allow user to download assests
///
_imcomp_ctrl.prototype.download_compare_asset = function(name) {
  var url = '';
  switch(name) {
    case 'homography':
      var data = this.m.now.compare_result.server_response.homography.join(',');
      var blob = new Blob([data], {type: 'text/csv;charset=utf-8'});
      this.save_data_to_local_file(blob, 'homography.csv');
      break;

    case 'difference_image':
      var url = this.m.now.compare_result.file1_file2_diff;
      break;
    case 'file1_region':
      var url = this.m.now.compare_result.file1.crop_url;
      break;
    case 'file2_region':
      var url = this.m.now.compare_result.file2.crop_url;
      break;
    case 'file2_transformed_region':
      var url = this.m.now.compare_result.file2.crop_tx_url;
      break;

    default:
      console.log('cannot download unknown asset : ' + name);
  }
  if(url !== '') {
    window.open(url, '_blank');
  }
}

_imcomp_ctrl.prototype.save_data_to_local_file = function(blob, local_filename) {
  var a      = document.createElement('a');
  a.href     = URL.createObjectURL(blob);
  a.target   = '_blank';
  a.download = local_filename;

  // simulate a mouse click event
  var event = new MouseEvent('click', {
    view: window,
    bubbles: true,
    cancelable: true
  });

  a.dispatchEvent(event);
}
