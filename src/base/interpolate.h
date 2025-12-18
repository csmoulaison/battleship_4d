#ifndef interpolate_h_INCLUDED
#define interpolate_h_INCLUDED

f32 clamp(f32 n, f32 min, f32 max);
f32 lerp(f32 a, f32 b, f32 t);
f32 smoothstep(f32 a, f32 b, f32 n);

#ifdef CSM_BASE_IMPLEMENTATION

f32 clamp(f32 n, f32 min, f32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

f32 lerp(f32 a, f32 b, f32 t)
{
	return (1.0f - t) * a + t * b;
}

f32 smoothstep(f32 a, f32 b, f32 n) {
	n = clamp((n - a) / (b - a), 0.0f, 1.0f);
	return n * n * (3.0f - 2.0f * n);
}

#endif // CSM_BASE_INTERPOLATION

#endif // interpolate_h_INCLUDED
