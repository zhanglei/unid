# unid
PHP unique ID generator, based on the Twitter Snowflake algorithm!

functions list:<br />

```php
<?php
// 1) Get the next unique ID.
string unid_next_id(void);

// 2) Change unique ID to timestamp.
int unid_get_time(string id);

// 3) Change unique ID to machine info.
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
[ukey]
unid.datacenter = integer
unid.twepoch = uint64
</code></pre>
