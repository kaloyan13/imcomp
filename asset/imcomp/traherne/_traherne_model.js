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
// @file        _traherne_model.js
// @description Model of the MVC design for user interface javascript
// @author      Abhishek Dutta <adutta@robots.ox.ac.uk>
// @date        Nov 2017
//
////////////////////////////////////////////////////////////////////////////////

function _traherne_model() {
  this.via = {};

  this.files = {};
  this.file_count = {};
  this.fid_to_index = {};
  this.index_to_fid = {};
  this.fid_to_upload_id = {};
  this.index_to_upload_id = {};

  this.upload = {};
  this.upload_scale = {};
  this.upload_status = {};

  this.compare_status = {};
}

_traherne_model.prototype.init = function( traherne_controller ) {
  this.c = traherne_controller;

  for( var type in this.c.type_list ) {
    this.reset_model_state(type);
  }

  this.set_via_config();
}

_traherne_model.prototype.reset_model_state = function( type ) {
  var via_panel = document.getElementById( this.c.type_list[type] + '_via_panel' );
  this.via[type] = new _via();
  this.via[type].init(via_panel);
  this.init_via_hooks(type);

  this.file_count[type] = 0;
}

// ensures that only one region is present at any given time
_traherne_model.prototype.init_via_hooks = function(type) {
  // only 1 region can exist in base
  if( type === 'base' ) {
    this.via[type].c.add_hook(this.via[type].c.hook.id.REGION_ADDED, function(param) {
      //console.log('hook: region added : fileid=' + param.fileid + ', rid=' + param.rid);

      // delete old region from image1
      if ( this.via[type].v.now.all_rid_list.length > 1 ) {
        var old_region = [ this.via[type].v.now.all_rid_list[0] ];
        this.via[type].c.region_delete(old_region);
      }

      if( this.file_count['base'] && this.file_count['comp'] ) {
        this.c.show_message('Now click the <span class="blue">Compare Base & Comp.</span> button to compare the region selected in base image and the corresponding matching region in the comp image.');
      }
    }.bind(this));
  }

  // no regions can be drawn in comp
  if( type === 'comp' ) {
    this.via[type].c.add_hook(this.via[type].c.hook.id.REGION_ADDED, function(param) {
      //console.log('hook: region added : fileid=' + param.fileid + ', rid=' + param.rid);

      // delete all regions from image1
      if ( this.via[type].v.now.all_rid_list.length ) {
        var regions = this.via[type].v.now.all_rid_list.slice(0);
        this.via[type].c.region_delete( regions );
      }

      this.c.show_message('Regions <span class="red">can only</span> be defined in the base image!');
    }.bind(this));
  }
}

_traherne_model.prototype.set_via_config = function() {
  // ensure that only one region is present at any given time in base image
  this.via['base'].c.add_hook(this.via['base'].c.hook.id.REGION_ADDED, function(param) {
    //console.log('hook: region added : fileid=' + param.fileid + ', rid=' + param.rid);
    if ( this.via['base'].v.now.all_rid_list.length > 1 ) {
      var old_region = [ this.via['base'].v.now.all_rid_list[0] ];
      this.via['base'].c.region_delete(old_region);
    }
  }.bind(this));

  // ensure that no regions are present in comp. image
  this.via['comp'].c.add_hook(this.via['comp'].c.hook.id.REGION_ADDED, function(param) {
    // delete all regions from comp
    if ( this.via['comp'].v.now.all_rid_list.length ) {
      var regions = this.via['comp'].v.now.all_rid_list.slice(0);
      this.via['comp'].c.region_delete( regions );
    }
  }.bind(this));
}

_traherne_model.prototype.clear_images = function( type ) {
  this.files[type] = [];

  this.file_count[type] = 0;
  this.fid_to_index[type] = {};
  this.index_to_fid[type] = {};
  this.fid_to_upload_id[type] = {};
  this.index_to_upload_id[type] = {};

  this.upload[type] = {};
  this.upload_status[type] = {};
  this.upload_scale[type] = {};
}

_traherne_model.prototype.add_images = function( type, files ) {
  this.clear_images(type);
  this.files[type] = Array.from(files);

  // sort files based on filename
  this.files[type].sort(
    function(a,b) {
      if( a.name < b.name ) {
        return -1;
      }
      if( a.name > b.name ) {
        return 1;
      }
      return 0;
    }
  );

  var n = this.files[type].length;
  var promises = [];
  for (var i=0; i < n; i++) {
    //('[' + i + '] Adding local file to ' + type + ' : ' + this.files[type][i].name);
    promises.push( this.via[type].m.add_file_local(this.files[type][i]) );
  }

  Promise.all(promises).then( function(result) {
    var n = result.length;

    // @todo: this can be avoided in Promise.all() can operate on this.upload[type]
    //var upload_promises = [];

    for( var i=0; i<n; i++ ) {
      var fid = result[i];
      if ( !fid.startsWith("Error") ) {
        // record the internal file-id in via
        this.fid_to_index[type][fid] = i;
        this.index_to_fid[type][i] = fid;
        this.file_count[type] = this.file_count[type] + 1;

        // upload image to server
        this.upload_status[type][i] = {};
        this.set_upload_status(type, i, '...', 'Queued for upload');
        this.upload[type][i] = this.upload_file(type, i);
        //upload_promises.push( this.upload[type][i] ); // @todo: avoid
      }
    }
    this.c.on_filelist_update(type);
    /*
    // @todo: err_callback() is not being invoked correctly.
    // I expect, error.length == 12, if err_callback() is invoked 12 times
    // but this is not happening. @fixme

    console.log(upload_promises.length);
    Promise.all( upload_promises ).then( function(result) {
      console.log('result count = ' + result.length + ', {' + result + '}');
      if( result.length !== 0 ) {
        this.c.append_message(' <span class="blue">[added ' + result.length + ' images]</span>');
      }
    }.bind(this), function(error) {
      console.log(error.length);
      if( error.length !== 0 ) {
        this.c.append_message(' <span class="red">[failed to add ' + error.length + ' images]</span>');
      }
    }.bind(this));
    */

  }.bind(this));
}

_traherne_model.prototype.get_filelist = function(type) {
  var filelist = [];
  var n = this.files[type].length;

  for( var i=0; i<n; i++ ) {
    filelist.push(this.files[type][i].name);
  }
  return filelist;
}

_traherne_model.prototype.set_upload_status = function(type, findex, status, msg) {
  //console.log('set_upload_status() : ' + type + ' : ' + findex + ' ' + status + ':' + msg);
  this.upload_status[type][findex].status = status;
  this.upload_status[type][findex].msg = msg;
  this.c.on_upload_status_update(type, findex);
}

_traherne_model.prototype.upload_file = function(type, findex) {
  return new Promise( function(ok_callback, err_callback) {
    var file_content = this.files[type][findex];
    var file_source = 'local';

    var filereader = new FileReader();

    filereader.addEventListener( 'load', function() {
      // filereader.result contains binary file data
      // we convert it to base64 format by creating an 'img' html element
      var img = new Image();
      img.addEventListener('load', function() {
        var scaled_dim = '';
        var is_scaled = false;
        var scaled_img_base64;
        this.upload_scale[type][findex] = 1.0;

        // actual image width and height
        var cw = img.naturalWidth;
        var ch = img.naturalHeight;

        // resize image (if required)
        if ( this.c.is_upload_resize_req(type, findex, cw, ch) ) {
          is_scaled = true;
          this.set_upload_status(type, findex, '...', 'Resizing image');

          // largest possible image width and height
          var lw = this.c.config.upload.MAX_IMG_DIM_PX;
          var lh = this.c.config.upload.MAX_IMG_DIM_PX;

          // transformed content's position, width and height
          var txw = cw;
          var txh = ch;

          var scale_width = lw / cw;
          txw = lw;
          txh = ch * scale_width;

          if ( txh > lh ) {
            var scale_height = lh / txh;
            txh = lh;
            txw = txw * scale_height;
          }
          txw = Math.floor(txw);
          txh = Math.floor(txh);
          var canvas = document.createElement('canvas');
          var ctx = canvas.getContext('2d');
          canvas.width = txw;
          canvas.height = txh;
          ctx.drawImage(img, 0, 0, txw, txh);

          scaled_img_base64 = canvas.toDataURL('image/jpeg', 0.8);
          scaled_dim = ' (scaled to ' + txw + 'x' + txh + ' pixels )';
          this.upload_scale[type][findex] = txw / cw;
        }

        this.set_upload_status(type, findex, '...', 'Uploading image');
        var uploader = new XMLHttpRequest();
        uploader.addEventListener('error', function(e) {
          err_callback(findex);
          var msg = 'Upload error!';
          console.log('uploader.event() == error');
          this.set_upload_status(type, findex, 'ERR', msg);
        }.bind(this));
        uploader.addEventListener('abort', function(e) {
          err_callback("_ERROR_");
          var msg = 'Upload abort!';
          console.log('aborted');
          this.set_upload_status(type, findex, 'ERR', msg);
        }.bind(this));

        uploader.addEventListener('load', function() {
          //console.log('response = [' + uploader.responseText + ']');
          try {
            var response_str = uploader.responseText;
            if ( response_str === '' ) {
              var msg = 'Error uploading image! [empty server response]';
              this.set_upload_status(type, findex, 'ERR', msg);
              err_callback(findex);
              console.log(msg);
            } else {
              var response = JSON.parse(response_str);
              if(response.fid) {
                var msg = 'File uploaded [' + response.fid + ']';
                this.set_upload_status(type, findex, 'OK', msg);
                ok_callback(response.fid);
              } else {
                console.log('Error uploading image!');
                var msg = 'Error uploading image!';
                this.set_upload_status(type, findex, 'ERR', msg);
                err_callback("_ERROR_");
                console.log(msg);
              }
            }
          }
          catch(e) {
            var msg = 'Error uploading image! [exception occured]';
            console.log('error except');
            this.set_upload_status(type, findex, 'ERR', msg);
            err_callback("_ERROR_");
          }
        }.bind(this));

        // upload image binary data
        var img_binary_data = filereader.result;
        if ( is_scaled ) {
          // convert image data in base64 format to binary data
          var content_type = scaled_img_base64.substr(5, scaled_img_base64.indexOf(';') - 5);
          var content      = scaled_img_base64.substr(scaled_img_base64.indexOf(',') + 1); // remove base64 header

          // source: https://stackoverflow.com/a/14988118/7814484
          var img_decoded_base64 = atob(content);
          var img_decoded_len = img_decoded_base64.length;
          img_binary_data = new ArrayBuffer(img_decoded_len);
          var uint8_view = new Uint8Array(img_binary_data);
          for ( var i = 0; i<img_decoded_len; i++ ) {
            uint8_view[i] = img_decoded_base64.charCodeAt(i);
          }
        }
        uploader.open('POST', this.c.config.imcomp_server_upload_uri , true);
        uploader.setRequestHeader('Content-Type', 'application/octet-stream');
        uploader.send(img_binary_data);
      }.bind(this), false);

      // convert image data array buffer to blob url so that
      // it can be read as image
      var img_blob = new Blob([filereader.result]);
      var url_creator = window.URL || window.webkitURL;
      var img_url = url_creator.createObjectURL(img_blob);
      img.src = img_url;
    }.bind(this));

    filereader.addEventListener( 'error', err_callback);
    filereader.addEventListener( 'abort', err_callback);
    if ( file_source === 'local' ) {
      //filereader.readAsDataURL( file_content );
      filereader.readAsArrayBuffer( file_content );
    } else {
      filereader.readAsText( new Blob([file_content]) );
    }
  }.bind(this));
}

///
/// Comparison of an image pair
///
_traherne_model.prototype.file_count = function(type) {
  var count = 0;
  for( var file in this.m.files[type] ) {
    if( this.m.files[type].hasOwnProperty(file) ) {
      count = count + 1;
    }
  }
  return count;
}

_traherne_model.prototype.set_compare_status = function(status, msg) {
  //console.log('set_compare_status() : ' + status + ':' + msg);
  if( status === 'OK' || status === 'ERR' ) {
    this.c.on_compare_end();
  }
  this.compare_status.status = status;
  this.compare_status.msg = msg;
  this.c.on_compare_status_update();
}

// c is an instance of _traherne_compare_instance
_traherne_model.prototype.compare_img_pair = function(c) {
  return new Promise( function(ok_callback, err_callback) {
    this.c.on_compare_start();
    var args = [];
    args.push('file1=' + c.upload_id1);
    args.push('file2=' + c.upload_id2);

    // resize the img1 region based on the scaling of upload image
    if( c.scale1 != 1.0 ) {
      for( var i=0; i<c.region1.length; i++ ) {
        c.region1[i] = Math.round( c.region1[i] * c.scale1 );
      }
    }
    args.push('region=' + c.region1.join(','));
    var algorithm_choice = document.getElementById('algorithm_choice');
    var algname = algorithm_choice.options[algorithm_choice.selectedIndex].value;
    args.push('algname=' + algname);

    var cr = new XMLHttpRequest();
    cr.addEventListener('timeout', function(e) {
      var msg = 'xhr timeout: [' + e + ']';
      this.set_compare_status('ERR', msg);
      err_callback();
    }.bind(this));
    cr.addEventListener('error', function(e) {
      var msg = 'xhr error: [' + e + ']';
      this.set_compare_status('ERR', msg);
      err_callback();
    }.bind(this));
    cr.addEventListener('abort', function(e) {
      var msg = 'xhr abort: [' + e + ']';
      this.set_compare_status('ERR', msg);
      err_callback();
    }.bind(this));

    cr.addEventListener('load', function() {
      var response_str = cr.responseText;
      try {
        c.response = JSON.parse(response_str).IMAGE_HOMOGRAPHY[0];
        if( c.response.status === 'OK' ) {
          var msg = 'finished comparison';
          this.set_compare_status('OK', msg);
          ok_callback(c);
        } else {
          var msg = 'comparison failed [' + c.response.status_message + ']';
          this.set_compare_status('ERR', msg);
          err_callback();
        }
      } catch(e) {
        var msg = 'malformed response from server [exception: ' + e + ']';
        this.set_compare_status('ERR', msg);
        err_callback();
      }
    }.bind(this));


    cr.open('POST', this.c.config.imcomp_server_compare_uri + '?' + args.join('&'));
    cr.timeout = 40000; // 20 sec
    cr.send();
  }.bind(this));
}
