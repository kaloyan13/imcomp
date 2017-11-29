function _imcomp_model() {
  this.about = {};
  this.about.name = "Image Comparator";
  this.about.shortname = "imcomp";
  this.about.version = "2.0.0";

  this.settings = {};
  this.settings.MAX_IMG_DIM = 1200; // in pixels

  this.server = {};
  this.server.imcompserver_url = "/imcomp/"
  this.server.fileserver_url   = "/imcomp/_upload";
  //this.server.imcompserver_url = "http://zeus.robots.ox.ac.uk/imcomp/"
  //this.server.fileserver_url   = "http://zeus.robots.ox.ac.uk/fs/";

  this.session = {};
  this.session.id = '';
  this.session.remote_files = {};
  this.session.remote_files_scale = {};

  this.now = {};
  this.now.files = {};
  this.now.compare_result = {};

  this.files = {};
  this.files.findex_fid_map = [];
  this.files.htmlelement    = [];
  this.files.metadata = [];
  this.files.content  = [];
  this.files.region   = []
  this.files.upload   = [];
}

_imcomp_model.prototype.init = function( imcomp_ctrl ) {
  //console.log("Initializing _imcomp_model ...");
  this.c = imcomp_ctrl;
  this.init_session();
}

_imcomp_model.prototype.init_session = function() {
}


_imcomp_model.prototype.add_file = function(filename, filesize, filesource, filetype, filecontent) {
  var findex = this.files.findex_fid_map.push("");
  findex = findex - 1;

  this.files.metadata[findex] = {};
  this.files.metadata[findex].filename = filename;
  this.files.metadata[findex].filesize = filesize;
  this.files.metadata[findex].filesource = filesource;
  this.files.metadata[findex].filetype = filetype;
  this.files.content[findex] = filecontent;
  this.files.region[findex] = [0,0,0,0]; // x0, y0, x1, y1

  this.files.upload[findex] = {};
  this.files.upload[findex].status = "";
  this.files.upload[findex].promise = this.upload_file(findex);
  return findex;
}

_imcomp_model.prototype.add_file_local = function(file) {
  var filename   = file.name;
  var filesize   = file.size;
  var filesource = "local";
  var filetype   = file.type.substr(0, 5);
  var filecontent= file
  return this.add_file(filename, filesize, filesource, filetype, filecontent);
}

_imcomp_model.prototype.upload_file = function(local_fid, file_content, file_source) {
  if( this.session.remote_files.hasOwnProperty(local_fid) ) {
    return false;
  }

  this.session.remote_files[local_fid] = new Promise(function(ok_callback, err_callback) {
    var filereader = new FileReader();
    this.c.update_file_upload_progress(local_fid, 'Reading local image file ...');

    filereader.addEventListener( 'load', function() {
      // filereader.result contains binary file data
      // we convert it to base64 format by creating an 'img' html element
      var img = new Image();
      img.addEventListener('load', function() {
        var scaled_dim = '';
        var is_scaled = false;
        var scaled_img_base64;
        this.session.remote_files_scale[local_fid] = 1.0;

        // resize image (if required)
        if ( img.naturalWidth > this.settings.MAX_IMG_DIM || img.naturalHeight > this.settings.MAX_IMG_DIM ) {
          is_scaled = true;
          this.c.update_file_upload_progress(local_fid, 'Resizing image (max. width or height < ' + this.settings.MAX_IMG_DIM + ' pixels) ...');

          // actual image width and height
          var cw = img.naturalWidth;
          var ch = img.naturalHeight;

          // largest possible image width and height
          var lw = this.settings.MAX_IMG_DIM;
          var lh = this.settings.MAX_IMG_DIM;

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
          this.session.remote_files_scale[local_fid] = txw / cw;
        }

        this.c.update_file_upload_progress(local_fid, 'Uploading image ...');
        var uploader = new XMLHttpRequest();
        uploader.addEventListener('error', function() {
          console.log('error : uploading image');
        }.bind(this));
        uploader.addEventListener('abort', function() {
          console.log('abort : uploading image');
        }.bind(this));

        uploader.addEventListener('load', function() {
          //console.log('response = [' + uploader.responseText + ']');
          try {
            var response_str = uploader.responseText;
            if ( response_str === '' ) {
              this.c.update_file_upload_progress(local_fid, '<span class="highlight">Error uploading image!</span>');
              err_callback();
            } else {
              var response = JSON.parse(response_str);
              if(response.fid) {
                //console.log('Uploaded file: sid=' + response.sid + ', remote-fid=' + response.fid + ', local-fid=' + local_fid);
                this.c.update_file_upload_progress(local_fid, 'Image uploaded.' + scaled_dim);
                ok_callback(response.fid);
              } else {
                this.c.update_file_upload_progress(local_fid, '<span class="highlight">Error uploading image! (unexpected response)</span>');
                err_callback();
              }
            }
          }
          catch(e) {
            this.c.update_file_upload_progress(local_fid, '<span class="highlight">Error uploading image!</span>');
            err_callback();
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
        uploader.open('POST', this.server.fileserver_url, true);
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
  return true;
}

// source: https://www.htmlgoodies.com/html5/tutorials/determine-an-images-type-using-the-javascript-filereader.html
_imcomp_model.prototype.get_file_mime_type = function(file_first_32bits) {
  var mime_type = 'unknown';
  switch(file_first_32bits) {
    case 1196314761: 
        mime_type = "image/png";
        break;
    case 944130375:
        mime_type = "image/gif";
        break;
    case 544099650:
        mime_type = "image/bmp";
        break;
    case -520103681:
        mime_type = "image/jpg";
        break;
  }
  return mime_type;
}

_imcomp_model.prototype.set_compare_result = function(result) {
  this.now.compare_result = {};
  this.now.compare_result.server_response = result;
  this.now.compare_result.file1 = {};
  this.now.compare_result.file1.local_fid  = result.file1_local_fid;
  this.now.compare_result.file1.remote_fid = result.file1_remote_fid;
  this.now.compare_result.file1.crop_url   = this.server.imcompserver_url + result.file1_crop;
  this.now.compare_result.file2 = {};
  this.now.compare_result.file2.local_fid  = result.file2_local_fid;
  this.now.compare_result.file2.remote_fid = result.file2_remote_fid;
  this.now.compare_result.file2.crop_url   = this.server.imcompserver_url + result.file2_crop;
  this.now.compare_result.file2.crop_tx_url= this.server.imcompserver_url + result.file2_crop_tx;

  this.now.compare_result.file1_file2_diff = this.server.imcompserver_url + result.file1_file2_diff;
  this.now.compare_result.file2_region = result.file2_region;

  // notify ctrl of compare_result update
  this.c.update_compare_result();
}
