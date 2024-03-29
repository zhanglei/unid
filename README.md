# unid
PHP unique ID generator, based on the Twitter Snowflake algorithm!

functions list:
---------------
```php
// 1) Get the next unique ID.
string unid_next_id(void);

// 2) Change unique ID to time stamp.
int unid_get_time(string id);

// 3) Change unique ID to worker id.
array unid_get_worker_id(string id);
```

example:
--------
```php
<?php
$id = unid_next_id();
echo $id;

$time = unid_get_time($id);
echo 'date time is: ' . date('Y-m-d H:i:s', $time);

$worker_id = unid_get_worker_id();
echo 'worker id is: ' . $worker_id;
?>
```


install:
--------
<pre><code>
$  cd ./unid
$  phpize
$  ./configure
$  make
$  sudo make install
</code></pre>


php.ini configure entries:
--------------------------
<pre><code>
[unid]
unid.datacenter = int
unid.twepoch = uint64
</code></pre>
