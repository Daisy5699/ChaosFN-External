#pragma once

namespace math
{
	struct f_vector_2d
	{
		double x , y;

		f_vector_2d( ) : x( 0 ) , y( 0 ) { }
		f_vector_2d( double x , double y ) : x( x ) , y( y ) { }

		auto operator+( const f_vector_2d & v ) const -> f_vector_2d { return f_vector_2d( x + v.x , y + v.y ); }
		auto operator-( const f_vector_2d & v ) const -> f_vector_2d { return f_vector_2d( x - v.x , y - v.y ); }
		auto operator*( double v ) const -> f_vector_2d { return f_vector_2d( x * v , y * v ); }
		auto operator/( double v ) const -> f_vector_2d { return f_vector_2d( x / v , y / v ); }

		auto distance( const f_vector_2d & v ) const -> double { return std::sqrt( std::pow( v.x - x , 2 ) + std::pow( v.y - y , 2 ) ); }
	};

	struct f_vector
	{
		double x , y , z;

		f_vector( ) : x( 0 ) , y( 0 ) , z( 0 ) { }
		f_vector( double x , double y , double z ) : x( x ) , y( y ) , z( z ) { }

		auto operator+( const f_vector & v ) const -> f_vector { return f_vector( x + v.x , y + v.y , z + v.z ); }
		auto operator-( const f_vector & v ) const -> f_vector { return f_vector( x - v.x , y - v.y , z - v.z ); }
		auto operator*( double v ) const -> f_vector { return f_vector( x * v , y * v , z * v ); }
		auto operator/( double v ) const -> f_vector { return f_vector( x / v , y / v , z / v ); }

		auto distance( const f_vector & v ) const -> double { return std::sqrt( std::pow( v.x - x , 2 ) + std::pow( v.y - y , 2 ) + std::pow( v.z - z , 2 ) ); }
		auto length( ) const -> double { return std::sqrt( x * x + y * y + z * z ); }
		inline double dot(const f_vector& v) const { return x * v.x + y * v.y + z * v.z; }
	};

	struct f_rotator
	{
		double pitch , yaw , roll;

		f_rotator( ) : pitch( 0 ) , yaw( 0 ) , roll( 0 ) { }
		f_rotator( double pitch , double yaw , double roll ) : pitch( pitch ) , yaw( yaw ) , roll( roll ) { }
	};

	struct f_matrix
	{
		float m [ 4 ][ 4 ];

		f_matrix( ) { memset( m , 0 , sizeof( m ) ); }
	};

	struct f_minimal_view_info
	{
		f_vector location; // 0x0000
		f_rotator rotation; // 0x0018
		float fov; // 0x0030
		float desired_fov; // 0x0034
		float first_person_fov; // 0x0038
		float first_person_scale; // 0x003C
		float ortho_width; // 0x0040

		// 0x0044 Bitfields / Pad
		uint8_t b_auto_calculate_ortho_planes : 1; // 0x0044 (Bit 0)
		uint8_t pad_45 [ 0x3 ]; // 0x0045

		float auto_plane_shift; // 0x0048

		// 0x004C Bitfields / Pad
		uint8_t b_update_ortho_planes : 1; // 0x004C (Bit 0)
		uint8_t b_use_camera_height_as_view_target : 1; // 0x004D (Bit 0)
		uint8_t pad_4e [ 0x2 ]; // 0x004E

		float ortho_near_clip_plane; // 0x0050
		float ortho_far_clip_plane; // 0x0054
		float perspective_near_clip_plane; // 0x0058
		float aspect_ratio; // 0x005C
		uint8_t pad_60 [ 0x8 ]; // 0x0060

		// 0x0068 Combined Bitfields
		uint8_t b_constrain_aspect_ratio : 1; // 0x0068 (Bit 0)
		uint8_t b_use_first_person_parameters : 1; // 0x0068 (Bit 1)
		uint8_t b_use_field_of_view_for_lod : 1; // 0x0068 (Bit 2)
		uint8_t pad_69 [ 0x3 ]; // 0x0069

		int32_t projection_mode; // 0x006C (Using int32_t to naturally cover the 1-byte enum + 3-byte pad at 0x6D)
		float post_process_blend_weight; // 0x0070
		uint8_t pad_74 [ 0xC ]; // 0x0074

		uint8_t post_process_settings [ 0x7B0 ]; // 0x0080 (Updated Opaque buffer size)
		f_vector_2d off_center_projection_offset; // 0x0830
		uint8_t pad_840 [ 0x70 ]; // 0x0840

		float overscan_resolution_fraction; // 0x08B0
		float crop_fraction; // 0x08B4
		uint8_t pad_8b8 [ 0x8 ]; // 0x08B8
		float asymmetric_crop_fraction[4]; // 0x08C0 (Make sure you have f_vector_4f defined, or use float[4])
	}; // Size: 0x08D0

	struct f_camera_cache_entry
	{
		float Timestamp; // 0x0 [Size: 0x4]
		uint8_t pad_4 [ 0xc ]; // 0x4
		f_minimal_view_info POV; // 0x10 [Size: 0x910]
	}; // Size: 0x0630

	struct alignas(16) f_transform
	{
		double Rotation[4]; // f_quat (x, y, z, w)
		double Translation[4]; // f_vector
		double Scale3D[4]; // f_vector

		f_matrix to_matrix_with_scale() const
		{
			f_matrix m;
			m.m[3][0] = static_cast<float>(Translation[0]);
			m.m[3][1] = static_cast<float>(Translation[1]);
			m.m[3][2] = static_cast<float>(Translation[2]);

			float x2 = static_cast<float>(Rotation[0] + Rotation[0]);
			float y2 = static_cast<float>(Rotation[1] + Rotation[1]);
			float z2 = static_cast<float>(Rotation[2] + Rotation[2]);
			float xx2 = static_cast<float>(Rotation[0]) * x2;
			float xy2 = static_cast<float>(Rotation[0]) * y2;
			float xz2 = static_cast<float>(Rotation[0]) * z2;
			float yy2 = static_cast<float>(Rotation[1]) * y2;
			float yz2 = static_cast<float>(Rotation[1]) * z2;
			float zz2 = static_cast<float>(Rotation[2]) * z2;
			float wx2 = static_cast<float>(Rotation[3]) * x2;
			float wy2 = static_cast<float>(Rotation[3]) * y2;
			float wz2 = static_cast<float>(Rotation[3]) * z2;

			m.m[0][0] = (1.0f - (yy2 + zz2)) * static_cast<float>(Scale3D[0]);
			m.m[1][0] = (xy2 - wz2) * static_cast<float>(Scale3D[1]);
			m.m[2][0] = (xz2 + wy2) * static_cast<float>(Scale3D[2]);
			m.m[0][1] = (xy2 + wz2) * static_cast<float>(Scale3D[0]);
			m.m[1][1] = (1.0f - (xx2 + zz2)) * static_cast<float>(Scale3D[1]);
			m.m[2][1] = (yz2 - wx2) * static_cast<float>(Scale3D[2]);
			m.m[0][2] = (xz2 - wy2) * static_cast<float>(Scale3D[0]);
			m.m[1][2] = (yz2 + wx2) * static_cast<float>(Scale3D[1]);
			m.m[2][2] = (1.0f - (xx2 + yy2)) * static_cast<float>(Scale3D[2]);

			m.m[0][3] = 0.0f;
			m.m[1][3] = 0.0f;
			m.m[2][3] = 0.0f;
			m.m[3][3] = 1.0f;

			return m;
		}
	};

	inline f_matrix matrix_multiplication(f_matrix pM1, f_matrix pM2)
	{
		f_matrix pOut;
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				pOut.m[i][j] = pM1.m[i][0] * pM2.m[0][j] +
					pM1.m[i][1] * pM2.m[1][j] +
					pM1.m[i][2] * pM2.m[2][j] +
					pM1.m[i][3] * pM2.m[3][j];
			}
		}
		return pOut;
	}

	inline f_matrix get_rotation_matrix(f_rotator rotation)
	{
		float rad_pitch = static_cast<float>(rotation.pitch * 3.14159265358979323846 / 180.0);
		float rad_yaw = static_cast<float>(rotation.yaw * 3.14159265358979323846 / 180.0);
		float rad_roll = static_cast<float>(rotation.roll * 3.14159265358979323846 / 180.0);

		float sp = sinf(rad_pitch);
		float cp = cosf(rad_pitch);
		float sy = sinf(rad_yaw);
		float cy = cosf(rad_yaw);
		float sr = sinf(rad_roll);
		float cr = cosf(rad_roll);

		f_matrix matrix;
		matrix.m[0][0] = cp * cy;
		matrix.m[0][1] = cp * sy;
		matrix.m[0][2] = sp;
		matrix.m[0][3] = 0.f;

		matrix.m[1][0] = sr * sp * cy - cr * sy;
		matrix.m[1][1] = sr * sp * sy + cr * cy;
		matrix.m[1][2] = -sr * cp;
		matrix.m[1][3] = 0.f;

		matrix.m[2][0] = -(cr * sp * cy + sr * sy);
		matrix.m[2][1] = cy * sr - cr * sp * sy;
		matrix.m[2][2] = cr * cp;
		matrix.m[2][3] = 0.f;

		matrix.m[3][0] = 0.f;
		matrix.m[3][1] = 0.f;
		matrix.m[3][2] = 0.f;
		matrix.m[3][3] = 1.f;

		return matrix;
	}

	inline bool world_to_screen(f_vector world_location, f_vector_2d& screen_location, f_minimal_view_info pov, int screen_width, int screen_height)
	{
		if (pov.fov <= 0.0f || pov.fov > 180.0f)
			return false;

		f_matrix m_matrix = get_rotation_matrix(pov.rotation);
		f_vector x_axis = f_vector(m_matrix.m[0][0], m_matrix.m[0][1], m_matrix.m[0][2]);
		f_vector y_axis = f_vector(m_matrix.m[1][0], m_matrix.m[1][1], m_matrix.m[1][2]);
		f_vector z_axis = f_vector(m_matrix.m[2][0], m_matrix.m[2][1], m_matrix.m[2][2]);

		f_vector v_delta_coordinates = world_location - pov.location;
		f_vector t_transform = f_vector( v_delta_coordinates.dot(y_axis),  v_delta_coordinates.dot(z_axis),  v_delta_coordinates.dot(x_axis) );

		if (t_transform.z < 1.0)
		{
			return false;
		}

		double fov_radians = tan(pov.fov * 3.14159265358979323846 / 360.0);
		double width_center = static_cast<double>(screen_width) / 2.0;
		double height_center = static_cast<double>(screen_height) / 2.0;

		screen_location.x = width_center + t_transform.x * (width_center / fov_radians) / t_transform.z;
		screen_location.y = height_center - t_transform.y * (width_center / fov_radians) / t_transform.z;

		if (screen_location.x < 0 || screen_location.x > screen_width || screen_location.y < 0 || screen_location.y > screen_height)
			return false;

		return true;
	}

}