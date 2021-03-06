///////////////////////// audio //////////////////////////

IGNORE_WARN(audio_debug)
IGNORE_WARN(audio_get_type)

// legacy
IGNORE_WARN(sound_volume)
IGNORE_WARN(sound_global_volume)
IGNORE_WARN(sound_fade)
IGNORE_WARN(sound_pan)
IGNORE_WARN(sound_background_tempo)
IGNORE_WARN(sound_set_search_directory)
IGNORE_WARN(sound_add)
IGNORE_WARN(sound_replace)
IGNORE_WARN(sound_delete)

// audio position
IGNORE_WARN(audio_emitter_create)
IGNORE_WARN(audio_emitter_exists)
IGNORE_WARN(audio_emitter_falloff)
IGNORE_WARN(audio_emitter_free)
IGNORE_WARN(audio_emitter_gain)
IGNORE_WARN(audio_emitter_get_gain)
IGNORE_WARN(audio_emitter_get_listener_mask)
IGNORE_WARN(audio_emitter_get_pitch)
IGNORE_WARN(audio_emitter_get_vx)
IGNORE_WARN(audio_emitter_get_vy)
IGNORE_WARN(audio_emitter_get_vz)
IGNORE_WARN(audio_emitter_get_x)
IGNORE_WARN(audio_emitter_get_y)
IGNORE_WARN(audio_emitter_get_z)
IGNORE_WARN(audio_emitter_pitch)
IGNORE_WARN(audio_emitter_position)
IGNORE_WARN(audio_emitter_set_listener_mask)
IGNORE_WARN(audio_emitter_velocity)
IGNORE_WARN(audio_falloff_set_model)
IGNORE_WARN(audio_get_listener_count)
IGNORE_WARN(audio_get_listener_info)
IGNORE_WARN(audio_get_listener_mask)
IGNORE_WARN(audio_get_master_gain)
IGNORE_WARN(audio_listener_get_data)
IGNORE_WARN(audio_listener_orientation)
IGNORE_WARN(audio_listener_position)
IGNORE_WARN(audio_listener_set_orientation)
IGNORE_WARN(audio_listener_set_position)
IGNORE_WARN(audio_listener_set_velocity)
IGNORE_WARN(audio_listener_velocity)
IGNORE_WARN(audio_play_sound_at)
IGNORE_WARN(audio_play_sound_on)
IGNORE_WARN(audio_set_listener_mask)
IGNORE_WARN(audio_set_master_gain)
IGNORE_WARN(audio_sound_get_listener_mask)
IGNORE_WARN(audio_sound_set_listener_mask)

// audio groups
IGNORE_WARN(audio_group_is_loaded)
IGNORE_WARN(audio_group_load)
IGNORE_WARN(audio_group_load_progress)
IGNORE_WARN(audio_group_name)
IGNORE_WARN(audio_group_set_gain)
IGNORE_WARN(audio_group_stop_all)
IGNORE_WARN(audio_group_unload)

// stream
IGNORE_WARN(audio_create_stream)
IGNORE_WARN(audio_destroy_stream)

// audio sync
IGNORE_WARN(audio_create_sync_group)
IGNORE_WARN(audio_destroy_sync_group)
IGNORE_WARN(audio_pause_sync_group)
IGNORE_WARN(audio_play_in_sync_group)
IGNORE_WARN(audio_resume_sync_group)
IGNORE_WARN(audio_stop_sync_group)
IGNORE_WARN(audio_sync_group_debug)
IGNORE_WARN(audio_sync_group_get_track_pos)
IGNORE_WARN(audio_sync_group_is_playing)
IGNORE_WARN(audio_start_sync_group)

// audio buffers
IGNORE_WARN(audio_create_buffer_sound)
IGNORE_WARN(audio_create_play_queue)
IGNORE_WARN(audio_free_buffer_sound)
IGNORE_WARN(audio_free_play_queue)
IGNORE_WARN(audio_get_recorder_count)
IGNORE_WARN(audio_get_recorder_info)
IGNORE_WARN(audio_queue_sound)
IGNORE_WARN(audio_start_recording)
IGNORE_WARN(audio_stop_recording)