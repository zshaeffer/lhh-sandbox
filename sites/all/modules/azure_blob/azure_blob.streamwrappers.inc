<?php

/**
 * @file
 * Class implementation of the stream wrapper to interface with Windows Azure.
 * 
 * Extends the Stream class of the Windows Azure SDK for PHP.
 * 
 * @author Damien Tournoud (Windows Azure Integration Developer), atuyo60
 */

require_once drupal_get_path('module', 'azure_blob') . '/lib/vendor/autoload.php';
use WindowsAzure\Common\ServicesBuilder;
use WindowsAzure\Blob\Models\CreateBlobOptions;
use WindowsAzure\Blob\Models\ListBlobsOptions;

/**
 * Implements the Drupal Stream Wrappers interface for Windows Azure Blob Storage.
 *
 * In addition to the base implementation from the Windows Azure SDK for PHP, this
 * implementation:
 *  - properly sets the MIME type of the objects when saving them
 *  - places all files in a pre-selected container
 *  - implements the Drupal additions to the stream wrapper interface
 */
class WindowsAzureBlobStreamWrapper implements DrupalStreamWrapperInterface {

  /**
   * List of valid clients with different stream wrappers
   * @var array
   */
  protected static $clients = array();
  
  /**
   * Blob service proxy client
   * @var BlobRestProxy
   */
  protected $client = NULL;
  
  /**
   * Instance URI (current filename)
   * @var string
   */
  protected $uri = NULL;
  
  /**
   * Temporary filename
   * @var string
   */
  protected $temp_filename = NULL;
  
  /**
   * Temporary file handle
   * @var string
   */
  protected $temp_file_handle = NULL;
  
  /**
   * Write mode?
   * @var boolean
   */
  protected $write_mode = false;
  
  /**
   * List of blobs
   * @var array
   */
  protected $blob_list = array();
  
  /**
   * List of known virtual directories
   * @var array 
   */
  protected $known_dirs = array();

  /**
   * {@inheritdoc}
   */
  public static function getMimeType($uri, $mapping = NULL) {
    return DrupalPublicStreamWrapper::getMimeType($uri, $mapping);
  }
  
  /**
   * Base implementation of setUri().
   * 
   * Set the absolute stream resource URI.
   * Generally is only called by the factory method.
   * 
   * @param string $uri
   *   A string containing the URI that should be used for this instance.
   */
  public function setUri($uri) {
    // Convert all back-slashes to forward-slash
    $this->uri = str_replace('\\', '/', $uri);
  }
  
  /**
   * Base implementation of getUri().
   * 
   * Returns the stream resource URI.
   * 
   * @return string
   *   The resource URI.
   */
  public function getUri() {
    return $this->uri;
  }
  
  /**
   * Base implementation of chmod().
   * 
   * Changes permissions of the resource.
   * 
   * @param int $mode
   *   Integer value for the permissions. Consult PHP chmod() for more information.
   * 
   * @return boolean
   *   Returns TRUE on success or FALSE on failure.
   */
  public function chmod($mode) {
    // @todo Implement setting of permissions in Windows Azure?
    return TRUE;
  }

  /**
   * Base implementation of realpath().
   * 
   * Returns canonical, absolute path of the resource.
   * 
   * @return string|false
   *   Returns a string with absolute pathname on success (implemented by core wrappers), 
   *   or FALSE on failure or if the registered wrapper does not provide an implementation.
   */
  public function realpath() {
    // @todo If called as temporary://, return a realpath?
    return FALSE;
  }

  /**
   * Gets the name of the directory from a given path.
   * 
   * This method is usually accessed through drupal_dirname(), 
   * which wraps around the normal PHP dirname() function, 
   * which does not support stream wrappers.
   * 
   * @param string $uri
   *   An optional URI.
   * 
   * @return string
   *   A string containing the directory name.
   */
  public function dirname($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    list($scheme, $target) = explode('://', $uri, 2);
    // Use getTarget() if location for writing is different from reading
    $dirname = dirname(trim($target, '\/'));

    if ($dirname === '.') {
      $dirname = '';
    }

    return $scheme . '://' . $dirname;
  }
  
  /**
   * Returns the local writable target of the resource within the stream.
   * 
   * This function should be used in place of calls to realpath() or 
   * similar functions when attempting to determine the location of a file. 
   * While functions like realpath() may return the location of a read-only file, 
   * this method may return a URI or path suitable for writing that is 
   * completely separate from the URI used for reading.
   * 
   * @param string $uri
   *   An optional URI.
   * 
   * @return string
   *   String representing a location suitable for writing of a file.
   */
  protected function getTarget($uri = NULL) {
    if (!isset($uri)) {
      $uri = $this->uri;
    }

    list($scheme, $target) = explode('://', $uri, 2);

    // Remove erroneous leading or trailing, forward-slashes and backslashes.
    return trim($target, '\/');
  }
  
  /**
   * This function should return a URL that can be embedded in a web page 
   * and accessed from a browser.
   * 
   * @return string
   *   A string containing a web accessible URL for the resource.
   */
  public function getExternalUrl() {
    if ($this->blobExists($this->uri)) {
      return $this->getRealExternalUrl();
    }
    else {
      $path = $this->getTarget();
      $parts = explode('/', $path);
      $first_part = array_shift($parts);
      
      // If the file is a styles derivative, treat it differently
      if ($first_part === 'styles') {
        return url('azure/generate/' . implode('/', $parts), array('absolute' => true));
      }
    }
    
    return drupal_not_found();
  }
  
  /**
   * Helper function to get the URL of a remote resource.
   * 
   * @return string
   *   A string containing a web accessible URL for the resource.
   */
  public function getRealExternalUrl() {
    return url('azure/remote/' . file_uri_scheme($this->uri) . '/' . $this->getTarget(), 
      array(
        'absolute' => TRUE, // Always generate absolute URLs
        'https' => TRUE,    // Allow HTTPs to be enforced
      ));
  }

  /**
   * Sends the blob data to the client.
   * 
   * @return boolean
   *   TRUE if download was successful, FALSE if there was an error.
   */
  public function downloadContent() {
    try {
      $blob = $this->getStorageClient($this->uri)->getBlob(
        $this->getContainerName($this->uri), 
        $this->getFileName($this->uri));

      $result = fpassthru($blob->getContentStream());
      
      if ($result !== FALSE) {
        return TRUE;
      }
      else {
        return FALSE;
      }
    }
    catch (Exception $ex) {
      return FALSE;
    }
  }
  
  /**
   * Build a storage client.
   * 
   * @param string $scheme
   *   Stream wrapper name.
   * 
   * @return BlobRestProxy
   */
  protected static function createStorageClient($scheme) {
    if (!isset(self::$clients[$scheme])) {
      // Obtain user configuration from database
      $configuration = ctools_export_crud_load('azure_blob', $scheme);

      if (isset($configuration)) {
        // @todo Use shared access signature to specify a specific blob endpoint
        $connection_string = 'DefaultEndpointsProtocol=https;AccountName=' .
          $configuration->account . ';AccountKey=' . $configuration->primary_key;
        
        self::$clients[$scheme] = 
          ServicesBuilder::getInstance()->createBlobService($connection_string);
        
        // Store other configuration properties in client object
        self::$clients[$scheme]->blob_container = $configuration->blob_container;
        self::$clients[$scheme]->cache_control = $configuration->cache_control;
      }
      else {
        throw new Exception(t('Invalid storage scheme "@stream"', array('@stream' => $scheme)));
      }
    }

    return self::$clients[$scheme];
  }

  /**
   * Retrieve storage client for this stream type.
   * 
   * @param string $uri
   * 
   * @return BlobRestProxy
   */
  protected function getStorageClient($uri = '') {
    if ($this->client === NULL) {
      $scheme = file_uri_scheme($uri);
      
      if ($scheme === FALSE) {
        throw new Exception(t('Could not parse URI %uri', array('%uri' => $uri)));
      }
      
      $this->client = self::createStorageClient($scheme);
      
      if (!$this->client) {
        throw new Exception(t('No clients registered for "@stream://"', 
          array('@stream' => $scheme)));
      }
    }

    return $this->client;
  }

  /**
   * Support for fopen(), file_get_contents(), file_put_contents() etc.
   * 
   * @param string $uri
   *   A string containing the URI to the file to open.
   * @param string $mode
   *   The file mode ("r", "wb" etc.).
   * @param integer $options
   *   A bit mask of STREAM_USE_PATH and STREAM_REPORT_ERRORS.
   * @param string $opened_path
   *   A string containing the path actually opened.
   * 
   * @return boolean
   *   Returns TRUE if file was opened successfully.
   */
  public function stream_open($uri, $mode, $options, &$opened_path) {
    $this->setUri($uri);
    
    try {
      $blob_name = $this->getFileName($this->uri);
      
      if (empty($blob_name)) {
        throw new Exception(t('Empty blob path name given. Has to be a full filename.'));
      }

      // Write mode?
      if (strpbrk($mode, 'wax+')) {
        $this->write_mode = TRUE;
      } 
      else {
        $this->write_mode = FALSE;
      }
      
      $result = FALSE;

      // If read/append, fetch the file
      if (!$this->write_mode || strpbrk($mode, 'ra+')) {
        $blob = $this->getStorageClient($this->uri)->
          getBlob(
            $this->getContainerName($this->uri),
            $this->getFileName($this->uri));
        
        $this->temp_file_handle = $blob->getContentStream();
        $result = TRUE;
      }
      else {
        $this->temp_filename = tempnam(sys_get_temp_dir(), 'azure');

        // Check the file can be opened
        $fh = @fopen($this->temp_filename, $mode);
        if ($fh !== FALSE) {
          fclose($fh);

          // Open temporary file handle
          $this->temp_file_handle = fopen($this->temp_filename, $mode);

          // Ok!
          $result = TRUE;
        }
      }
      
      // If file opened successfully and currently in write mode,
      // ensure that the parent directory exists
      if ($result && $this->write_mode) {
        if (!$this->isDirectory($this->dirname())) {
          return FALSE;
        }
      }
      
      return $result;
    }
    catch (Exception $ex) {
      // The stream_open() function should not raise any exception.
      return FALSE;
    }
  }
  
  /**
   * Base implementation of stream_lock().
   * 
   * Support for flock().
   * 
   * @param int $operation
   *   - LOCK_SH to acquire a shared lock (reader)
   *   - LOCK_EX to acquire an exclusive lock (writer)
   *   - LOCK_UN to release a lock (shared or exclusive)
   *   - LOCK_NB if you don't want flock() to block while locking. (not supported on Windows)
   * 
   * @return boolean
   *   Always returns TRUE at present.
   */
  public function stream_lock($operation) {
    if (in_array($operation, array(LOCK_SH, LOCK_EX, LOCK_UN, LOCK_NB))) {
      return flock($this->temp_file_handle, $operation);
    }
  
    return TRUE;
  }

  /**
   * Closes the stream. Support for fclose().
   * 
   * @return boolean
   *   TRUE if stream was successfully closed.
   */
  public function stream_close() {
    // Prevent timeout when uploading
    drupal_set_time_limit(0);

    @fclose($this->temp_file_handle);

    // Upload the file?
    if ($this->write_mode) {
      $client = $this->getStorageClient($this->uri);
      $container = $this->getContainerName($this->uri);
      
      // Make sure the container exists
      if (!$this->containerExists($this->uri)) {
        // Create default private container if none exists
        $client->createContainer($container);
      }

      // Upload the file
      try {
        // Blob creation properties
        $create_options = new CreateBlobOptions();
        $create_options->setContentType($this->getMimeType($this->uri));
        $create_options->setCacheControl(!empty($client->cache_control) ? 
          $client->cache_control : 'no-cache, must-revalidate, post-check=0, pre-check=0');
        
        $client->createBlockBlob(
          $container,
          $this->getFileName($this->uri),
          fopen($this->temp_filename, 'r'),
          $create_options);
      }
      catch (Exception $ex) {
        $this->cleanup();

        // Upload failed
        return FALSE;
      }
    }

    $this->cleanup();
    
    return TRUE;
  }
  
  /**
   * Helper function to cleanup after the stream is closed.
   */
  private function cleanup() {
    if ($this->temp_filename) {
      @unlink($this->temp_filename);
    }
    unset($this->client);
  }
  
  /**
   * Support for fread(), file_get_contents() etc.
   *
   * @param int $count
   *   Maximum number of bytes to be read.
   * 
   * @return string|false
   *   The string that was read, or FALSE in case of an error.
   */
  public function stream_read($count) {
    if (!$this->temp_file_handle) {
      return FALSE;
    }

    return fread($this->temp_file_handle, $count);
  }
  
  /**
   * Support for fwrite(), file_put_contents() etc.
   * 
   * @param string $data
   *   The string to be written.
   * 
   * @return int
   *   The number of bytes written
   */
  public function stream_write($data) {
    if (!$this->temp_file_handle) {
      return 0;
    }

    $len = strlen($data);
    fwrite($this->temp_file_handle, $data, $len);
    
    return $len;
  }

  /**
   * End of the stream? Support for feof().
   *
   * @return boolean
   *   TRUE if end-of-file has been reached.
   */
  public function stream_eof() {
    if (!$this->temp_file_handle) {
      return TRUE;
    }

    return feof($this->temp_file_handle);
  }

  /**
   * What is the current read/write position of the stream?
   *
   * @return int
   *   The current offset in bytes from the beginning of file.
   */
  public function stream_tell() {
    return ftell($this->temp_file_handle);
  }

  /**
   * Update the read/write position of the stream.
   *
   * @param integer $offset
   *   The byte offset to got to.
   * @param integer $whence
   *   SEEK_SET, SEEK_CUR, or SEEK_END
   * 
   * @return boolean
   *   TRUE on success.
   */
  public function stream_seek($offset, $whence) {
    if (!$this->temp_file_handle) {
        return FALSE;
    }

    return (fseek($this->temp_file_handle, $offset, $whence) === 0);
  }

  /**
   * Flush current cached stream data to storage.
   *
   * @return boolean
   *   TRUE if data was successfully stored (or there was no data to store).
   */
  public function stream_flush() {
    $result = fflush($this->temp_file_handle);

     // Upload the file?
    if ($this->write_mode) {
      $client = $this->getStorageClient($this->uri);
      $container = $this->getContainerName($this->uri);
      
      // Make sure the container exists
      if (!$this->containerExists($this->uri)) {
        // Create default private container
        $client->createContainer($container);
      }

      // Upload the file
      try {
        $client->createBlockBlob(
          $container,
          $this->getFileName($this->uri),
          $this->temp_file_handle);
      }
      catch (Exception $ex) {
        $this->cleanup();

        // Upload failed
        return FALSE;
      }
    }

    return $result;
  }
  
  /**
   * Returns data array of stream variables
   *
   * @return array
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   */
  public function stream_stat() {
    if (!$this->temp_file_handle) {
      return FALSE;
    }

    return $this->url_stat($this->uri, 0);
  }

  /**
   * Attempt to delete the item.
   * 
   * @param string $uri
   *   A string containing the URI to the resource to delete.
   * 
   * @return boolean
   *   TRUE if resource was successfully deleted.
   */
  public function unlink($uri) {
    $this->setUri($uri);
    
    // unlink() should never throw an exception
    try {
      $this->getStorageClient($this->uri)->deleteBlob(
        $this->getContainerName($this->uri),
        $this->getFileName($this->uri));

      // Clear the file status cache
      clearstatcache(TRUE, $uri);
      
      return TRUE;
    }
    catch (Exception $ex) {
      return FALSE;
    }
  }
  
  /** 
   * Attempt to rename the item
   * 
   * @param string $path_from
   *   The URI to the file to rename.
   * @param string $path_to
   *   The new URI for file.
   * 
   * @return boolean
   *   TRUE if file was successfully renamed.
   */
  public function rename($path_from, $path_to) {
    $src_container = $this->getContainerName($path_from);
    $dst_container = $this->getContainerName($path_to);
    
    if ($src_container !== $dst_container) {
      throw new Exception(t('Container name cannot be changed.'));
    }
    
    $src_filename = $this->getFileName($path_from);
    $dst_filename = $this->getFileName($path_to);

    // No name difference
    if ($src_filename === $dst_filename) {
      return TRUE;
    }

    $this->getStorageClient($path_from)->copyBlob(
      $dst_container,
      $dst_filename,
      $src_container,
      $src_filename
    );
    
    $this->getStorageClient($path_from)->deleteBlob(
      $src_container,
      $src_filename
    );

    // Clear the file status cache for the affected paths
    clearstatcache(TRUE, $path_from);
    clearstatcache(TRUE, $path_to);

    return TRUE;
  }

  /**
   * Return array of URL variables.
   * 
   * @param string $uri
   *   A string containing the URI to get information about.
   * @param integer $flags
   *   A bit mask of STREAM_URL_STAT_LINK and STREAM_URL_STAT_QUIET.
   * 
   * @return array
   *   An array with file status, or FALSE in case of an error - see fstat()
   *   for a description of this array.
   */
  public function url_stat($uri, $flags) {
    $this->setUri($uri);
    
    // Default values
    $stat = array(
      'dev' => 0,
      'ino' => 0,
      'mode' => 0666,
      'nlink' => 0,
      'uid' => 0,
      'gid' => 0,
      'rdev' => 0,
      'size' => 0,
      'atime' => 0,
      'mtime' => 0,
      'ctime' => 0,
      'blksize' => 0,
      'blocks' => 0,
    );

    $path = $this->getTarget();
    
    if ($path === '' || isset($this->known_dirs[$path])) {
      // It is a directory
      $stat['mode'] |= 040777;
    }
    else {
      try {
        $blob_properties = $this->getStorageClient($this->uri)->
          getBlobProperties(
            $this->getContainerName($this->uri),
            $this->getFileName($this->uri));

        // Set the modification time and last modified to the Last-Modified header
        $lastmodified = $blob_properties->getProperties()->getLastModified()->format('U');
        $stat['mtime'] = $lastmodified;
        $stat['ctime'] = $lastmodified;
        $stat['size'] = $blob_properties->getProperties()->getContentLength();

        // Entry is a regular file with group access
        $stat['mode'] = 0100000|660;
      }
      catch (Exception $ex) {
        // Unexisting file... check if it is a directory
        if ($this->isDirectory($this->uri)) {
          // It is a directory
          $stat['mode'] |= 0040777;
        }
        else {
          // File really does not exist
          return FALSE;
        }
      }
    }
    
    // Last access time
    $stat['atime'] = time();
    // Return both numeric and associative values
    return array_values($stat) + $stat;
  }

  /**
   * Helper function to check if path is a directory.
   * 
   * @param string $uri
   * 
   * @return boolean
   *   TRUE if path is a directory.
   */
  protected function isDirectory($uri) {
    $client = $this->getStorageClient($uri);
    $container = $this->getContainerName($uri);
    
    // Ensure container exists before proceeding
    if (!$this->containerExists($uri)) {
      // Create default private container if container does not exist
      $client->createContainer($container);
    }
    
    $target = $this->getFileName($uri);

    if ($target === '') {
      // Root directory
      return TRUE;
    }
    
    // Ensure all back-slashes are converted to forward-slashes
    $path = str_replace('\\', '/', $target);

    try {
      // Check if the directory placeholder exists
      $client->getBlob(
        $container, 
        $path . '/.placeholder');
      
      return TRUE;
    }
    catch (Exception $ex) {
      // No placeholder
    }
    
    $blob_options = new ListBlobsOptions();
    $blob_options->setPrefix($path . '/');
    $blob_options->setDelimiter('/');
    $blob_options->setMaxResults(1);
    
    // Check if there are blobs in the directory
    $child_blobs = $client->listBlobs(
      $container,
      $blob_options);

    if (is_array($child_blobs)) {
      // Create the placeholder
      $client->createBlockBlob(
        $container,
        $path . '/.placeholder',
        '');
      
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Creates a new directory.
   * 
   * @param string $uri
   *   A string containing the URI to the directory to create.
   * @param int $mode
   *   Permission flags - see mkdir().
   * @param int $options
   *   A bit mask of STREAM_REPORT_ERRORS and STREAM_MKDIR_RECURSIVE.
   * 
   * @return boolean
   *   TRUE if directory was successfully created.
   */
  public function mkdir($uri, $mode, $options) {
    $this->setUri($uri);
    
    $client = $this->getStorageClient($this->uri);
    $container = $this->getContainerName($this->uri);
    
    // Check if container exists
    if (!$this->containerExists($this->uri)) {
      // Create default private container if none exists
      $client->createContainer($container);
    }

    // Create the placeholder for a virtual directory in the container
    $client->createBlockBlob(
      $container,
      $this->getTarget() . '/.placeholder',
      '');
    
    return TRUE;
  }

  /**
   * Remove a directory.
   * 
   * @param string $uri
   *   A string containing the URI to the directory to delete.
   * @param int $options
   *   A bit mask of STREAM_REPORT_ERRORS.
   * 
   * @return boolean
   *   TRUE if directory was successfully removed.
   */
  public function rmdir($uri, $options) {
    $this->setUri($uri);

    $path = $this->getTarget();
    
    $blob_options = new ListBlobsOptions();
    $blob_options->setPrefix(!empty($path) ? $path . '/' : '');
    $blob_options->setDelimiter('/');
    // Only need 2 results to determine if directory is empty
    $blob_options->setMaxResults(2);

    // Check if the "directory" is not empty, i.e. it contains
    // at least 2 objects, the placeholder and one other.
    $child_blobs = $this->getStorageClient($this->uri)->listBlobs(
      $this->getContainerName($this->uri),
      $blob_options);

    if (count($child_blobs) > 1) {
      // The directory is not empty, cannot remove it.
      return FALSE;
    }
    else {
      $this->unlink(rtrim($this->uri, '/') . '/.placeholder');
      return TRUE;
    }
  }

  /**
   * Attempt to open a directory.
   * 
   * @param string $uri
   *   A string containing the URI to the directory to open.
   * @param int $options
   *   Unknown (not documented).
   * 
   * @return boolean
   *   TRUE on success.
   */
  public function dir_opendir($uri, $options) {
    $this->setUri($uri);
    
    $path = $this->getTarget();
    
    $blob_options = new ListBlobsOptions();
    $blob_options->setPrefix(!empty($path) ? $path . '/' : '');
    $blob_options->setDelimiter('/');
    
    $blobs = $this->getStorageClient($this->uri)->listBlobs(
      $this->getContainerName($this->uri),
      $blob_options);

    if (is_array($blobs)) {
      $this->blob_list = array();
      $this->known_dirs = array();

      // Process the results.
      foreach ($blobs as $blob) {
        if (!empty($path)) {
          // Strip the directory name
          $blob_name = preg_replace(
            '/^' . preg_quote($path . '/', '/') . '/', '', $blob->getName());
        }
        else {
          $blob_name = $blob->getName();
        }

        // @todo Find replacement for isPrefix in new SDK
        /*if ($blob->isPrefix) {
          // Strip the trailing slash.
          $blob_name = rtrim($blob_name, '/');

          // Record that this prefix is a "directory", to optimize stream_stat().
          $this->known_dirs[] = $blob->getName();
        }*/

        // Keep track of all the blobs
        $this->blob_list[] = $blob_name;
      }

      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Return the next filename in the directory.
   * 
   * @return boolean
   *   The next filename, or FALSE if there are no more files in the directory.
   */
  public function dir_readdir() {
    $object = current($this->blob_list);
    if ($object !== FALSE) {
      next($this->blob_list);
      return $object;
    }
    return FALSE;
  }
  
  /**
   * Reset the directory pointer.
   *
   * @return boolean
   *   TRUE on success.
   */
  public function dir_rewinddir() {
    reset($this->blob_list);
    return TRUE;
  }

  /**
   * Close a directory.
   *
   * @return boolean
   *   TRUE on success.
   */
  public function dir_closedir() {
    $this->blob_list = array();
    return TRUE;
  }
  
  /**
   * Retrieve name of the container.
   * 
   * @param string $uri
   * 
   * @return string
   *   Container name
   */
  protected function getContainerName($uri) {
    return $this->getStorageClient($uri)->blob_container;
  }

  /**
   * Retrieve name of the blob.
   * 
   * @param string $uri
   * 
   * @return string
   *   Blob name
   */
  protected function getFileName($uri) {
    return $this->getTarget($uri);
  }
  
  /**
   * Helper function to check if container exists.
   * 
   * @param string $uri
   * 
   * @return boolean
   *   TRUE if container exists.
   */
  protected function containerExists($uri) {
    try {
      $this->getStorageClient($uri)->getContainerProperties(
        $this->getContainerName($uri));
      
      return TRUE;
    }
    catch (Exception $ex) {
      return FALSE;
    }
    
    /*$options = new ListContainersOptions();
    $options->setPrefix($this->getContainerName($uri));

    $containers = $this->getStorageClient($uri)->listContainers($options);

    return count($containers->getContainers());*/
  }
  
  /**
   * Helper function to check if blob exists.
   * 
   * @param string $uri
   * 
   * @return boolean
   *   TRUE if blob exists.
   */
  protected function blobExists($uri) {
    try {
      $this->getStorageClient($uri)->getBlobProperties(
          $this->getContainerName($uri),
          $this->getFileName($uri));

      return TRUE;
    }
    catch (Exception $ex) {
      return FALSE;
    }
  }
}
