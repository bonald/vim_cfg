
tbl_mirror_t*
mirror_get_session(uint32 sessionid)
{
    tbl_mirror_key_t mirror_key;

    mirror_key.id = sessionid;
    return tbl_mirror_get_mirror(&mirror_key);
}
