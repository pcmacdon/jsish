/*
=!EXPECTSTART!=
=!EXPECTEND!=
*/

try {
	try {
	        throw({a:1});
	} catch (b) {
		throw({b:2});
	} finally {
	}
} catch (b) {
} finally {

}
